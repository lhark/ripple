#include <stdlib.h>
#include <iostream>
#include "inf2705.h"
#include "constants.h"
#include "FBO.h"
#include "Packets.h"
#include "util.h"

#define SOL 1

enum {LocAttrib, LocUniform}; /* Shader location type */
enum {
    RWMP_SHADER,
    APD_SHADER,
    DPO_SHADER,
    DMM_SHADER,
    DT_SHADER,
    RAA_SHADER,
    NB_SHADERS
}; /* Shaders stages */

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLuint progRasterizeWaveMeshPosition;
GLuint progAddPacketDisplacement;
GLuint progDisplayPacketOutlined;
GLuint progDisplayMicroMesh;
GLuint progDisplayTerrain;
GLuint progRenderAA;
GLint locVertex = -1;
GLint locNormal = -1;
GLint locTexCoord = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locmatrNormale = -1;
GLint loclaTexture = -1;
// Locations for RasterizeWaveMeshPosition shader
GLint locVertexRWMP = -1;
GLint locTexCoordRWMP = -1;
GLint locmatrModelRWMP = -1;
GLint locmatrVisuRWMP = -1;
GLint locmatrProjRWMP = -1;
GLint locmatrNormaleRWMP = -1;
// Locations for AddPacketDisplacement shader
GLuint locPosAPD = -1;
GLuint locAtt1APD = -1;
GLuint locAtt2APD = -1;
GLuint locmatrModelAPD = -1;
GLuint locmatrVisuAPD = -1;
GLuint locmatrProjAPD = -1;
GLuint locTexAPD = -1;
// Locations for DisplayPacketOutlined shader
GLuint locPosDPO = -1;
GLuint locAtt1DPO = -1;
GLuint locAtt2DPO = -1;
GLuint locmatrModelDPO = -1;
GLuint locmatrVisuDPO = -1;
GLuint locmatrProjDPO = -1;
GLuint locTexDPO = -1;
// Locations for DisplayMicroMesh shader
GLuint locVertexDMM = -1;
GLuint locTexCoordDMM = -1;
GLuint locmatrModelDMM = -1;
GLuint locmatrVisuDMM = -1;
GLuint locmatrProjDMM = -1;
GLuint locTerrainTexDMM = -1;
GLuint locWaterPosTexDMM = -1;
GLuint locWaterHeightTexDMM = -1;
// Locationss for DisplayTerrain shader
GLuint locVertexDT = -1;
GLuint locmatrModelDT = -1;
GLuint locmatrVisuDT = -1;
GLuint locmatrProjDT = -1;
GLuint locTerrainTexDT = -1;
// Locations for RenderAA shader
GLuint locVertexRAA = -1;
GLuint locTexCoordRAA = -1;
GLuint locmatrModelRAA = -1;
GLuint locmatrVisuRAA = -1;
GLuint locmatrProjRAA = -1;
GLuint locTexRAA = -1;

GLuint indLightSource;
GLuint indFrontMaterial;
GLuint indLightModel;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[NB_SHADERS];
GLuint vbo[5];
GLuint ubo[4];
GLuint vbosQuad[2]; /* Vertex & Tex Coord */
GLuint vboPacket;
GLuint vboHeightField;
GLuint vboDMM[2]; /* Vertex & Index */

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;
MatricePipeline matrProjWide;

// les formes
FormeSphere *sphere = NULL, *sphereLumi = NULL;
FormeTheiere *theiere = NULL;
FormeTore *tore = NULL;
FormeCylindre *cylindre = NULL;
FormeCylindre *cone = NULL;

// variables pour définir le point de vue
double thetaCam = 180.0;        // angle de rotation de la caméra (coord. sphériques)
double phiCam = 0.0;          // angle de rotation de la caméra (coord. sphériques)
double distCam = 0.0;         // distance (coord. sphériques)
glm::vec3 cameraPos = glm::vec3(10.17, 22.13, 59.49);
float movementIncr = 0.8;
/* Movement direction */
float goF = 0; /* Forward */
float goR = 0; /* Right */
float goB = 0; /* Back */
float goL = 0; /* Left */
float goU = 0; /* Up */
float goD = 0; /* Down */

// variables d'état
bool enPerspective = false;   // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = true;     // le modèle est en mouvement/rotation automatique ou non
bool afficheAxes = false;      // indique si on affiche les axes
GLenum modePolygone = GL_FILL; // comment afficher les polygones

// FBOs
FBO *posFBO;
FBO *heightFBO;
FBO *aaFBO;

// Terrain mesh size
const int terrainW = 1024;
const int terrainH = 1024;

// Wave Packets
Packets *packets;
int packetBudget = 100000;
/* Wave packets:
 * vec4: xy = position, zw = direction
 * vec4: x = amplitude, y = wavelength, z = phase offset, w = enveloppe size
 * vec4: for rendering x = center of wave bending circle*/
GLfloat packetData[PACKET_GPU_BUFFER_SIZE * 3 * 4];

int nIndexDMM = 0;

bool debug = false;
int printonce = 0;

////////////////////////////////////////
// déclaration des variables globales //
////////////////////////////////////////

/* Heightmap for the terrain */
GLuint texTerrain = 0;

/* Forward declarations */
void displayPacketOutlined(int count);
void addPacketDisplacement(int count);
void displayTerrain();
void displayMicroMesh();


void verifierAngles()
{
   if ( thetaCam > 360.0 )
      thetaCam -= 360.0;
   else if ( thetaCam < 0.0 )
      thetaCam += 360.0;

   const GLdouble MINPHI = -90.0, MAXPHI = 90.0;
   if ( phiCam > MAXPHI )
      phiCam = MAXPHI;
   else if ( phiCam < MINPHI )
      phiCam = MINPHI;
}

void calculerPhysique( )
{
   verifierAngles();
}


void updatePackets()
{
    // Compute wave packets
    if (enmouvement)
        packets->AdvectWavePackets(INIT_WAVE_SPEED);

    if (!debug)
        heightFBO->CommencerCapture();
    int displayedPackets = 0;
    int packetChunk =0;
    /* Standard wave packets */
    for (int i = 0; i < packets->m_usedPackets; ++i) {
        int pk = packets->m_usedPacket[i];
        /* Test for 3rd vertex (sliding point) */
        if (!packets->m_packet[pk].use3rd) {
            /* Position */
            packetData[packetChunk++] = packets->m_packet[pk].midPos.x();
            packetData[packetChunk++] = packets->m_packet[pk].midPos.y();
            /* Direction */
            packetData[packetChunk++] = packets->m_packet[pk].travelDir.x();
            packetData[packetChunk++] = packets->m_packet[pk].travelDir.y();
            /* Att1 */
            packetData[packetChunk++] = packets->m_packet[pk].ampOld;
            packetData[packetChunk++] = 2.0f * M_PI / packets->m_packet[pk].k;
            packetData[packetChunk++] = packets->m_packet[pk].phase;
            packetData[packetChunk++] = packets->m_packet[pk].envelope;
            /* Att2 */
            packetData[packetChunk++] = packets->m_packet[pk].bending;
            packetData[packetChunk++] = 0.0;
            packetData[packetChunk++] = 0.0;
            packetData[packetChunk++] = 0.0;
            displayedPackets++;
            if (packetChunk >= PACKET_GPU_BUFFER_SIZE * 3 * 4) {
                glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
                /* TODO Use Buffer mapping for better performance */
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(packetData), packetData);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                addPacketDisplacement(displayedPackets);
                displayedPackets = 0;
                packetChunk = 0;
            }
        }
    }
    printonce++;
    // printf("PacketData[0] = %f , %f\n", packetData[0], packetData[1]);
    /* Ghost packets */
    for (int i = 0; i < packets->m_usedGhosts; ++i) {
        int pk = packets->m_usedGhost[i];
        /* Position */
        packetData[packetChunk++] = packets->m_ghostPacket[pk].pos.x();
        packetData[packetChunk++] = packets->m_ghostPacket[pk].pos.y();
        /* Direction */
        packetData[packetChunk++] = packets->m_ghostPacket[pk].dir.x();
        packetData[packetChunk++] = packets->m_ghostPacket[pk].dir.y();
        /* Att1 */
        packetData[packetChunk++] = packets->m_ghostPacket[pk].ampOld;
        packetData[packetChunk++] = 2.0f * M_PI / packets->m_ghostPacket[pk].k;
        packetData[packetChunk++] = packets->m_ghostPacket[pk].phase;
        packetData[packetChunk++] = packets->m_ghostPacket[pk].envelope;
        /* Att2 */
        packetData[packetChunk++] = packets->m_ghostPacket[pk].bending;
        packetData[packetChunk++] = 0.0;
        packetData[packetChunk++] = 0.0;
        packetData[packetChunk++] = 0.0;
        displayedPackets++;
        if (packetChunk >= PACKET_GPU_BUFFER_SIZE * 3 * 4) {
            glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(packetData), packetData);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            addPacketDisplacement(displayedPackets);
            displayedPackets = 0;
            packetChunk = 0;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(packetData), packetData);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    addPacketDisplacement(displayedPackets);
    if (!debug)
        heightFBO->TerminerCapture();
}


void chargerTextures()
{
   unsigned char *pixels;
   GLsizei largeur, hauteur;
   if ( ( pixels = ChargerImage( WATER_TERRAIN_FILE, largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &texTerrain );
      glBindTexture( GL_TEXTURE_2D, texTerrain );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
   }

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
}


/* Create program and link it
 * Input : filenames for vertex, geometry and fragment shader, or NULL*/
GLuint createShader(const GLchar *shaders[3])
{
    GLuint prog;
    GLenum shaderType[3] = {
        GL_VERTEX_SHADER,
        GL_GEOMETRY_SHADER,
        GL_FRAGMENT_SHADER
    };
    prog = glCreateProgram();
    for (int i = 0; i < 3; ++i) {
        if (shaders[i] != NULL) {
            printf("Compiling %s ...\n", shaders[i]);
            const GLchar *shaderStr = ProgNuanceur::lireNuanceur(shaders[i]);
            if (shaderStr == NULL)
                break;
            GLuint s = glCreateShader(shaderType[i]);
            glShaderSource( s, 1, &shaderStr, NULL );
            glCompileShader( s );
            glAttachShader( prog, s );
            ProgNuanceur::afficherLogCompile( s );
            delete [] shaderStr;
        }
    }
    glLinkProgram(prog);
    ProgNuanceur::afficherLogLink(prog);
    return prog;
}


GLuint
getloc(GLuint prog, const GLchar *name, const int type)
{
	GLuint loc;
	switch (type) {
	case LocAttrib:
		loc = glGetAttribLocation(prog, name);
		break;
	case LocUniform:
		loc = glGetUniformLocation(prog, name);
		break;
	}
	if (loc == -1)
		fprintf(stderr, "Cannot find location for %s\n", name);
	return loc;
}


void chargerNuanceurs()
{
   // charger le nuanceur de base
   {
      // créer le programme
      progBase = glCreateProgram();

      // attacher le nuanceur de sommets
      {
         GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesSommetsMinimal, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( progBase, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
      }
      // attacher le nuanceur de fragments
      {
         GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
         glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesFragmentsMinimal, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( progBase, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
      }

      // faire l'édition des liens du programme
      glLinkProgram( progBase );

      ProgNuanceur::afficherLogLink( progBase );
      // demander la "Location" des variables
      locVertexBase = getloc( progBase, "Vertex", LocAttrib );
      locColorBase = getloc( progBase, "Color", LocAttrib );
      locmatrModelBase = getloc( progBase, "matrModel", LocUniform );
      locmatrVisuBase = getloc( progBase, "matrVisu", LocUniform );
      locmatrProjBase = getloc( progBase, "matrProj", LocUniform );
   }

   // Load RasterizeWaveMeshPosition shader
   {
      // créer le programme
      const GLchar *shaders[3] = {"rasterizeWaveMeshPosition.vert", NULL, "rasterizeWaveMeshPosition.frag"};
      progRasterizeWaveMeshPosition = createShader(shaders);
      // demander la "Location" des variables
      locVertexRWMP = getloc( progRasterizeWaveMeshPosition, "Vertex" , LocAttrib);
      locTexCoordRWMP = getloc( progRasterizeWaveMeshPosition, "TexCoord" , LocAttrib);
      locmatrModelRWMP = getloc( progRasterizeWaveMeshPosition, "matrModel" , LocUniform);
      locmatrVisuRWMP = getloc( progRasterizeWaveMeshPosition, "matrVisu" , LocUniform);
      locmatrProjRWMP = getloc( progRasterizeWaveMeshPosition, "matrProj" , LocUniform);
   }

   // Load AddPacketDisplacement shader
   {
      // créer le programme
      const GLchar *shaders[3] = {"addPacketDisplacement.vert", "addPacketDisplacement.geom", "addPacketDisplacement.frag"};
      progAddPacketDisplacement = createShader(shaders);
      // demander la "Location" des variables
      locPosAPD = getloc( progAddPacketDisplacement, "Pos" , LocAttrib);
      locAtt1APD = getloc( progAddPacketDisplacement, "Att1" , LocAttrib);
      locAtt2APD = getloc( progAddPacketDisplacement, "Att2" , LocAttrib);
      locmatrModelAPD = getloc( progAddPacketDisplacement, "matrModel" , LocUniform);
      locmatrVisuAPD = getloc( progAddPacketDisplacement, "matrVisu" , LocUniform);
      locmatrProjAPD = getloc( progAddPacketDisplacement, "matrProj" , LocUniform);
   }

   // Load DisplayPacketOutlined shader
   {
      const GLchar *shaders[3] = {"displayPacketOutlined.vert", "displayPacketOutlined.geom", "displayPacketOutlined.frag"};
      progDisplayPacketOutlined = createShader(shaders);
      locPosDPO = getloc( progDisplayPacketOutlined, "Pos" , LocAttrib);
      locAtt1DPO = getloc( progDisplayPacketOutlined, "Att1" , LocAttrib);
      locAtt2DPO = getloc( progDisplayPacketOutlined, "Att2" , LocAttrib);
      locmatrModelDPO = getloc( progDisplayPacketOutlined, "matrModel" , LocUniform);
      locmatrVisuDPO = getloc( progDisplayPacketOutlined, "matrVisu" , LocUniform);
      locmatrProjDPO = getloc( progDisplayPacketOutlined, "matrProj" , LocUniform);
      locTexDPO = getloc( progDisplayPacketOutlined, "tex" , LocUniform);
   }

   // Load DisplayMicroMesh shader
   {
      const GLchar *shaders[3] = {"displayMicroMesh.vert", "displayMicroMesh.geom", "displayMicroMesh.frag"};
      progDisplayMicroMesh = createShader(shaders);
      locVertexDMM = getloc( progDisplayMicroMesh, "Vertex" , LocAttrib);
      locTexCoordDMM = getloc( progDisplayMicroMesh, "TexCoord" , LocAttrib);
      locmatrModelDMM = getloc( progDisplayMicroMesh, "matrModel" , LocUniform);
      locmatrVisuDMM = getloc( progDisplayMicroMesh, "matrVisu" , LocUniform);
      locmatrProjDMM = getloc( progDisplayMicroMesh, "matrProj" , LocUniform);
      locTerrainTexDMM = getloc( progDisplayMicroMesh, "terrain" , LocUniform);
      locWaterPosTexDMM = getloc( progDisplayMicroMesh, "waterPos" , LocUniform);
      locWaterHeightTexDMM = getloc( progDisplayMicroMesh, "waterHeight" , LocUniform);
   }

   // Load DisplayTerrain shader
   {
      const GLchar *shaders[3] = {"displayTerrain.vert", NULL, "displayTerrain.frag"};
      progDisplayTerrain = createShader(shaders);
      locVertexDT = getloc( progDisplayTerrain, "Vertex" , LocAttrib);
      locmatrModelDT = getloc( progDisplayTerrain, "matrModel" , LocUniform);
      locmatrVisuDT = getloc( progDisplayTerrain, "matrVisu" , LocUniform);
      locmatrProjDT = getloc( progDisplayTerrain, "matrProj" , LocUniform);
      locTerrainTexDT = getloc( progDisplayTerrain, "terrain" , LocUniform);
   }

   // Load RenderAA shader
   {
      const GLchar *shaders[3] = {"renderAA.vert", NULL, "RenderAA.frag"};
      progRenderAA = createShader(shaders);
      locVertexRAA = getloc( progRenderAA, "Vertex" , LocAttrib);
      locTexCoordRAA = getloc( progRenderAA, "TexCoord" , LocAttrib);
      locmatrModelRAA = getloc( progRenderAA, "matrModel" , LocUniform);
      locmatrVisuRAA = getloc( progRenderAA, "matrVisu" , LocUniform);
      locmatrProjRAA = getloc( progRenderAA, "matrProj" , LocUniform);
      locTexRAA = getloc( progRenderAA, "tex" , LocUniform);
   }
}


// Create a screen space quad
void initQuad()
{
   GLfloat vertices[3*2*2] = {
      -1.0, -1.0,    1.0, -1.0,   -1.0,  1.0,
       1.0, -1.0,    1.0,  1.0,   -1.0,  1.0
   };
   GLfloat texcoords[3*2*2] = {
      0.0, 1.0,   1.0, 1.0,   0.0, 0.0,
      1.0, 1.0,   1.0, 0.0,   0.0, 0.0
   };

   // allouer les objets OpenGL
   /* TODO Add support for RenderAA shader */
   glGenBuffers( 2, vbosQuad );

   /* Prepare VAO for RasterizeWaveMeshPosition shader */
   glBindVertexArray(vao[RWMP_SHADER]);
   // Bind vertices VBO
   glBindBuffer( GL_ARRAY_BUFFER, vbosQuad[0] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
   glVertexAttribPointer( locVertexRWMP, 2, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locVertexRWMP);
   // Bind texture coord VBO
   glBindBuffer( GL_ARRAY_BUFFER, vbosQuad[1] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW );
   glVertexAttribPointer( locTexCoordRWMP, 2, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locTexCoordRWMP);

   /* Prepare VAO for RenderAA shader */
   glBindVertexArray(vao[RAA_SHADER]);
   // Bind vertices VBO
   glBindBuffer( GL_ARRAY_BUFFER, vbosQuad[0] );
   glVertexAttribPointer( locVertexRAA, 2, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locVertexRAA);
   // Bind texture coord VBO
   glBindBuffer( GL_ARRAY_BUFFER, vbosQuad[1] );
   glVertexAttribPointer( locTexCoordRAA, 2, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locTexCoordRAA);
}


// Create the buffer to store wave packets
void initPacketMesh()
{
    glGenBuffers(1, &vboPacket);

    glBindVertexArray(vao[APD_SHADER]);
    glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
    glBufferData(GL_ARRAY_BUFFER, sizeof(packetData), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(locPosAPD, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)0);
    glVertexAttribPointer(locAtt1APD, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)(4*sizeof(GLfloat)));
    glVertexAttribPointer(locAtt2APD, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)(8*sizeof(GLfloat)));
    glEnableVertexAttribArray(locPosAPD);
    glEnableVertexAttribArray(locAtt1APD);
    glEnableVertexAttribArray(locAtt2APD);

    glBindVertexArray(vao[DPO_SHADER]);
    glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
    /* No need to initialize the buffer a second time */
    glVertexAttribPointer(locPosDPO, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)0);
    glVertexAttribPointer(locAtt1DPO, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)(4*sizeof(GLfloat)));
    glVertexAttribPointer(locAtt2DPO, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)(8*sizeof(GLfloat)));
    glEnableVertexAttribArray(locPosDPO);
    glEnableVertexAttribArray(locAtt1DPO);
    glEnableVertexAttribArray(locAtt2DPO);

    glBindVertexArray(0);
}


void initHeightFieldMesh()
{
    GLfloat *data = new GLfloat[terrainW * terrainH * 6 * 2];
    /* TODO Benchmark usefullness of parallel for */
    #pragma omp parallel for
    for (int y=0; y<terrainH; ++y) {
        for (int x=0; x<terrainW; ++x) {
            data[(y*terrainW + x) * 12 + 0] = 2.0 * (float)(x) / terrainW - 1.0;
            data[(y*terrainW + x) * 12 + 1] = 2.0 * (float)(y) / terrainH - 1.0;
            data[(y*terrainW + x) * 12 + 2] = 2.0 * (float)(x) / terrainW - 1.0;
            data[(y*terrainW + x) * 12 + 3] = 2.0 * (float)(y+1) / terrainH - 1.0;
            data[(y*terrainW + x) * 12 + 4] = 2.0 * (float)(x+1) / terrainW - 1.0;
            data[(y*terrainW + x) * 12 + 5] = 2.0 * (float)(y) / terrainH - 1.0;
            data[(y*terrainW + x) * 12 + 6] = 2.0 * (float)(x) / terrainW - 1.0;
            data[(y*terrainW + x) * 12 + 7] = 2.0 * (float)(y+1) / terrainH - 1.0;
            data[(y*terrainW + x) * 12 + 8] = 2.0 * (float)(x+1) / terrainW - 1.0;
            data[(y*terrainW + x) * 12 + 9] = 2.0 * (float)(y+1) / terrainH - 1.0;
            data[(y*terrainW + x) * 12 + 10] = 2.0 * (float)(x+1) / terrainW - 1.0;
            data[(y*terrainW + x) * 12 + 11] = 2.0 * (float)(y) / terrainH - 1.0;
        }
    }

    glGenBuffers(1, &vboHeightField);
    glBindVertexArray(vao[DT_SHADER]);
    glBindBuffer(GL_ARRAY_BUFFER, vboHeightField);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * terrainW * terrainH * 12, data, GL_STATIC_DRAW);
    glVertexAttribPointer(locVertexDT, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(locVertexDT);

    glBindVertexArray(0);
    delete [] data;
}


// initialisation d'openGL
void initialiser()
{
   // Create FBOs
   posFBO = new FBO();
   heightFBO = new FBO();
   aaFBO = new FBO();

   // Create Packets
   packets = new Packets(packetBudget);

   // couleur de l'arrière-plan
   glClearColor( 0.5, 0.6, 0.8, 0.0 );

   // activer les etats openGL
   glEnable( GL_DEPTH_TEST );

   // charger les textures
   chargerTextures();

   // allouer les UBO pour les variables uniformes
   glGenBuffers( 4, ubo );
   glGenVertexArrays(NB_SHADERS, vao);

   // charger les nuanceurs
   chargerNuanceurs();

   // Initialize VBOs
   initQuad();
   initPacketMesh();
   initHeightFieldMesh();
}

void conclure()
{
   glUseProgram( 0 );
   glDeleteVertexArrays( NB_SHADERS, vao );
   glDeleteBuffers( 4, vbo );
   glDeleteBuffers( 4, ubo );
   delete sphere;
   delete sphereLumi;
   delete theiere;
   delete tore;
   delete cylindre;
   delete cone;
   delete posFBO;
   delete heightFBO;
   delete aaFBO;
   delete packets;
}

void rasterizeWaveMeshPosition()
{
    glUseProgram(progRasterizeWaveMeshPosition);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glUniformMatrix4fv( locmatrProjRWMP, 1, GL_FALSE, matrProjWide );
    glUniformMatrix4fv( locmatrVisuRWMP, 1, GL_FALSE, matrVisu );
    glUniformMatrix4fv( locmatrModelRWMP, 1, GL_FALSE, matrModel );
    glBindVertexArray(vao[RWMP_SHADER]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
}

void displayPacketOutlined(int count)
{
    glUseProgram(progDisplayPacketOutlined);
    glBindVertexArray(vao[DPO_SHADER]);
    glUniformMatrix4fv(locmatrModelDPO, 1, GL_FALSE, matrModel);
    glUniformMatrix4fv(locmatrVisuDPO, 1, GL_FALSE, matrVisu);
    glUniformMatrix4fv(locmatrProjDPO, 1, GL_FALSE, matrProj);
    glDrawArrays(GL_POINTS, 0, count);
    glBindVertexArray(0);
    glUseProgram(0);
}


void addPacketDisplacement(int count)
{
    glUseProgram(progAddPacketDisplacement);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glBindVertexArray(vao[APD_SHADER]);
    glUniformMatrix4fv(locmatrModelAPD, 1, GL_FALSE, matrModel);
    glUniformMatrix4fv(locmatrVisuAPD, 1, GL_FALSE, matrVisu);
    glUniformMatrix4fv(locmatrProjAPD, 1, GL_FALSE, matrProjWide);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    glDrawArrays(GL_POINTS, 0, count);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glUseProgram(0);
}


void displayTerrain()
{
    glUseProgram(progDisplayTerrain);
    glBindVertexArray(vao[DT_SHADER]);
    glUniformMatrix4fv(locmatrModelDT, 1, GL_FALSE, matrModel);
    glUniformMatrix4fv(locmatrVisuDT, 1, GL_FALSE, matrVisu);
    glUniformMatrix4fv(locmatrProjDT, 1, GL_FALSE, matrProj);
    glActiveTexture(GL_TEXTURE0); /* Default value, can be omitted */
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glUniform1i(locTerrainTexDT, 0);
    glDrawArrays(GL_TRIANGLES, 0, terrainW * terrainH * 12);
    glBindVertexArray(0);
    glUseProgram(0);
}


void displayMicroMesh()
{
    glUseProgram(progDisplayMicroMesh);
    glBindVertexArray(vao[DMM_SHADER]);
    glUniformMatrix4fv(locmatrModelDMM, 1, GL_FALSE, matrModel);
    glUniformMatrix4fv(locmatrVisuDMM, 1, GL_FALSE, matrVisu);
    glUniformMatrix4fv(locmatrProjDMM, 1, GL_FALSE, matrProj);
    /* Setup textures */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texTerrain);
    glUniform1i(locTerrainTexDMM, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, posFBO->GetRGBTex());
    glUniform1i(locWaterPosTexDMM, 1);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, heightFBO->GetRGBTex());
    glUniform1i(locWaterHeightTexDMM, 2);

    glDrawElements(GL_TRIANGLES, nIndexDMM, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void afficherModele()
{
   // Dessiner le modèle
   matrModel.PushMatrix(); {
      /* Create texture terrain positions */
      posFBO->CommencerCapture();
      rasterizeWaveMeshPosition();
      posFBO->TerminerCapture();

      updatePackets();
      glClearColor( 0.5, 0.6, 0.8, 0.0 );
      displayTerrain();
      if (!debug)
          displayMicroMesh();
   } matrModel.PopMatrix();
}


// fonction d'affichage
void FenetreTP::afficherScene()
{
   // effacer l'ecran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   matrProj.Perspective( 30.0, (GLdouble)largeur_ / (GLdouble)hauteur_, 0.5, 4000.0 );
   matrProjWide.Perspective( 60.0, (GLdouble)largeur_ / (GLdouble)hauteur_, 0.1, 15000.0 );
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   //matrVisu.LookAt( 0.0, 3.0, distCam,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0 );
   // appliquer les rotations
   glm::vec3 view = glm::vec3(0.0, 0.0, 1.0);
   view = glm::rotate(view, (GLfloat)glm::radians(phiCam), glm::vec3(-1.0, 0.0, 0.0));
   view = glm::rotate(view, (GLfloat)glm::radians(thetaCam), glm::vec3(0.0, 1.0, 0.0));
   glm::vec3 forward = glm::normalize(glm::vec3(view.x, 0.0, view.z));
   glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
   glm::vec3 right = glm::normalize(glm::cross(forward, up));
   cameraPos += movementIncr * ((goF - goB) * forward + (goR - goL) * right + (goU - goD) * up);
   matrVisu.LookAt( cameraPos, cameraPos + view, glm::vec3(0.0, 1.0, 0.0));

   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes( 1.0 );

   afficherModele();
}

// fonction de redimensionnement de la fenêtre graphique
void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
   std::cout << "Resizing to " << w << "×" << h << std::endl;
   glViewport( 0, 0, w, h );
   posFBO->Liberer();
   posFBO->Init(WAVETEX_WIDTH_FACTOR * w, WAVETEX_HEIGHT_FACTOR * h);
   heightFBO->Liberer();
   heightFBO->Init(WAVETEX_WIDTH_FACTOR * w, WAVETEX_HEIGHT_FACTOR * h);
   aaFBO->Liberer();
   aaFBO->Init(AA_OVERSAMPLE_FACTOR * w, AA_OVERSAMPLE_FACTOR * h);

   /* Create/resize display mesh */
   int meshW = WAVEMESH_WIDTH_FACTOR * w; /*Ça fait un gros mouton */
   int meshH = WAVEMESH_HEIGHT_FACTOR * h;
   GLfloat *mesh = new GLfloat[meshW * meshH * 2];
   nIndexDMM = 3 * 2 * (meshW - 1) * (meshH -1);
   GLuint *index = new GLuint[nIndexDMM];
   #pragma omp parallel for
   for (int y = 0; y < meshH; ++y) {
       for (int x = 0; x < meshW; ++x) {
           mesh[(y * meshW + x) * 2 + 0] = (x + 0.5) / meshW;
           mesh[(y * meshW + x) * 2 + 1] = (y + 0.5) / meshH;
       }
   }
   #pragma omp parallel for
   for (int y = 0; y < meshH - 1; ++y) {
       for (int x = 0; x < meshW - 1; ++x) {
           index[(y * (meshW - 1) + x) * 6 + 0] = y * meshW + x;
           index[(y * (meshW - 1) + x) * 6 + 1] = y * meshW + x + 1;
           index[(y * (meshW - 1) + x) * 6 + 2] = (y + 1) * meshW + x;
           index[(y * (meshW - 1) + x) * 6 + 3] = y * meshW + x + 1;
           index[(y * (meshW - 1) + x) * 6 + 4] = (y + 1) * meshW + x + 1;
           index[(y * (meshW - 1) + x) * 6 + 5] = (y + 1) * meshW + x;
       }
   }
   glGenBuffers(2, vboDMM);
   glBindVertexArray(vao[DMM_SHADER]);
   glBindBuffer(GL_ARRAY_BUFFER, vboDMM[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * meshW * meshH * 2, mesh, GL_STATIC_DRAW);
   glVertexAttribPointer(locVertexDMM, 2, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(locVertexDMM);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboDMM[1]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * nIndexDMM, index, GL_STATIC_DRAW);
   glBindVertexArray(0);

   delete [] mesh;
   delete [] index;
}


/* Key up */
void FenetreTP::clavierRelache( TP_touche touche)
{
    switch ( touche )
    {
        /* Mouvement camera */
    case TP_z: /* Forward */
        goF = 0;
        break;
    case TP_s: /* Backward */
        goB = 0;
        break;
    case TP_q: /* Left */
        goL = 0;
        break;
    case TP_d: /* Right */
        goR = 0;
        break;
    case TP_ESPACE:
        goU = 0;
        break;
    case TP_CONTROLEGAUCHE:
        goD = 0;
        break;
    default:
        break;
    }
}

// fonction de gestion du clavier
void FenetreTP::clavier( TP_touche touche )
{
   // traitement des touches q et echap
   switch ( touche )
   {
   case TP_ECHAP:
      quit();
      break;

   /* Mouvement camera */
   case TP_z: /* Forward */
      goF = 1;
      break;
   case TP_s: /* Backward */
      goB = 1;
      break;
   case TP_q: /* Left */
      goL = 1;
      break;
   case TP_d: /* Right */
      goR = 1;
      break;
   case TP_ESPACE: /* Up */
      goU = 1;
      break;
   case TP_CONTROLEGAUCHE: /* Down */
      goD = 1;
      break;

   case TP_h:
      debug = !debug;
      printf("Debug is %s!\n", (debug ? "active" : "inactive"));
      break;

   case TP_x: // Activer/désactiver l'affichage des axes
      afficheAxes = !afficheAxes;
      std::cout << "// Affichage des axes ? " << ( afficheAxes ? "OUI" : "NON" ) << std::endl;
      break;

   case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
      chargerNuanceurs();
      std::cout << "// Recharger nuanceurs" << std::endl;
      break;

   case TP_r:
      packets->Reset();
      printonce = 0;
      printf("Reseting...\n");
      break;

   case TP_e:
       // Send one wave front "One ping only."
       packets->CreateCircularWavefront(0.0, 0.0, 2.0, 0.2, 1.0, 10000);
      printonce = 0;
       printf("Sending a circular wavefront...\n");
      break;

   case TP_f:
       // Send one wave front "One ping only."
       packets->CreateLinearWavefront(0.0, 0.0, 1.0, 0.0, 2.0, 0.2, 1.0, 10000);
      printonce = 0;
       printf("Sending a linear wavefront...\n");
      break;

   case TP_a:
       // Send one packet
       packets->CreateSpreadingPacket(0.0, 0.0, 1.0, 0.0, 1.0, 2.0, 0.2, 1.0, 10000);
      printonce = 0;
       printf("Sending a spreading packet...\n");
      break;

   case TP_0:
      thetaCam = 0.0; phiCam = 0.0; distCam = 30.0; // placer les choses afin d'avoir une belle vue
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
      break;

   case TP_p: // Permuter la rotation automatique du modèle
      enmouvement = !enmouvement;
      printf("Simulation %s\n", (enmouvement ? "running" : "stopped"));
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      imprimerTouches();
      break;
   }

}

// fonction callback pour un clic de souris
int dernierX = 0; // la dernière valeur en X de position de la souris
int dernierY = 0; // la derniere valeur en Y de position de la souris
static enum { deplaceCam, deplaceSpotDirection, deplaceSpotPosition } deplace = deplaceCam;
static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
   pressed = ( state == TP_PRESSE );
   if ( pressed )
   {
      // on vient de presser la souris
      dernierX = x;
      dernierY = y;
      switch ( button )
      {
      case TP_BOUTON_GAUCHE: // Tourner l'objet
         deplace = deplaceCam;
         break;
      case TP_BOUTON_MILIEU: // Modifier l'orientation du spot
         deplace = deplaceSpotDirection;
         break;
      case TP_BOUTON_DROIT: // Déplacer la lumière
         deplace = deplaceSpotPosition;
         break;
      }
   }
   else
   {
      // on vient de relâcher la souris
   }
}


void FenetreTP::sourisWheel(int x, int y) {}


// fonction de mouvement de la souris
void FenetreTP::sourisMouvement( int x, int y )
{
   if ( pressed )
   {
      int dx = x - dernierX;
      int dy = y - dernierY;
      switch ( deplace )
      {
      case deplaceCam:
         thetaCam -= dx / 9.0;
         phiCam   -= dy / 9.0;
         break;
      }

      dernierX = x;
      dernierY = y;

      verifierAngles();
   }
}

int main( int argc, char *argv[] )
{
   // créer une fenêtre
   FenetreTP fenetre( "Ripple" );

   // allouer des ressources et définir le contexte OpenGL
   initialiser();

   bool boucler = true;
   while ( boucler )
   {
      // mettre à jour la physique
      calculerPhysique( );

      // affichage
      fenetre.afficherScene();
      fenetre.swap();

      // récupérer les événements et appeler la fonction de rappel
      boucler = fenetre.gererEvenement();
   }

   // détruire les ressources OpenGL allouées
   conclure();

   return 0;
}
