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
GLint locTexRWMP = -1;
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
GLuint locTexDMM = -1;
// Locationss for DisplayTerrain shader
GLuint locVertexDT = -1;
GLuint locTexCoordDT = -1;
GLuint locmatrModelDT = -1;
GLuint locmatrVisuDT = -1;
GLuint locmatrProjDT = -1;
GLuint locTexDT = -1;
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
GLuint indvarsUnif;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[NB_SHADERS];
GLuint vbo[5];
GLuint ubo[4];
GLuint vbosQuad[2];
GLuint vboPacket;

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;

// les formes
FormeSphere *sphere = NULL, *sphereLumi = NULL;
FormeTheiere *theiere = NULL;
FormeTore *tore = NULL;
FormeCylindre *cylindre = NULL;
FormeCylindre *cone = NULL;

// variables pour définir le point de vue
double thetaCam = 0.0;        // angle de rotation de la caméra (coord. sphériques)
double phiCam = 0.0;          // angle de rotation de la caméra (coord. sphériques)
double distCam = 0.0;         // distance (coord. sphériques)

// variables d'état
bool enPerspective = false;   // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = false;     // le modèle est en mouvement/rotation automatique ou non
bool afficheAxes = true;      // indique si on affiche les axes
GLenum modePolygone = GL_FILL; // comment afficher les polygones

// FBOs
FBO *posFBO;
FBO *heightFBO;
FBO *aaFBO;

// Wave Packets
Packets *packets;
int packetBudget = 10000;
/* Wave packets:
 * vec4: xy = position, zw = direction
 * vec4: x = amplitude, y = wavelength, z = phase offset, w = enveloppe size
 * vec4: for rendering x = center of wave bending circle*/
GLfloat packetData[PACKET_GPU_BUFFER_SIZE * 3 * 4];

////////////////////////////////////////
// déclaration des variables globales //
////////////////////////////////////////

// partie 1: illumination
int modele = 1;                  // le modèle à afficher

// partie 3: texture
GLuint texTerrain = 0;
GLuint textureECHIQUIER = 0;

// définition des lumières
struct LightSourceParameters
{
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   glm::vec4 position;
   glm::vec3 spotDirection;
   float spotExposant;
   float spotAngle;            // ([0.0,90.0] ou 180.0)
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;
} LightSource[1] = { { glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                       glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                       glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                       glm::vec4( 4, 1, 15, 1.0 ),
                       glm::vec3( -5.0, -2.0, -10.0 ),
                       1.0,       // l'exposant du cône
                       15.0,      // l'angle du cône du spot
                       1., 0., 0. } };

// définition du matériau
struct MaterialParameters
{
   glm::vec4 emission;
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   float shininess;
} FrontMaterial = { glm::vec4( 0.0, 0.0, 0.0, 1.0 ),
                    glm::vec4( 0.1, 0.1, 0.1, 1.0 ),
                    glm::vec4( 1.0, 0.1, 1.0, 1.0 ),
                    glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                    100.0 };

struct LightModelParameters
{
   glm::vec4 ambient; // couleur ambiante
   int localViewer;   // doit-on prendre en compte la position de l'observateur? (local ou à l'infini)
   int twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel = { glm::vec4(0,0,0,1), false, false };

struct
{
   // partie 1: illumination
   int typeIllumination;     // 0:Lambert, 1:Gouraud, 2:Phong
   int utiliseBlinn;         // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
   int utiliseDirect;        // indique si on utilise un spot style Direct3D ou OpenGL
   int afficheNormales;      // indique si on utilise les normales comme couleurs (utile pour le débogage)
   // partie 3: texture
   int texnumero;            // numéro de la texture appliquée
   int utiliseCouleur;       // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
   int afficheTexelNoir;     // un texel noir doit-il être affiché 0:noir, 1:mi-coloré, 2:transparent?
} varsUnif = { 2, false, false, false,
               0, true, 0 };
// ( En glsl, les types 'bool' et 'int' sont de la même taille, ce qui n'est pas le cas en C++.
// Ci-dessus, on triche donc un peu en déclarant les 'bool' comme des 'int', mais ça facilite la
// copie directe vers le nuanceur où les variables seront bien de type 'bool'. )


/* Forward declarations */
void displayPacketOutlined(int count);
void addPacketDisplacement(int count);


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
   if ( enmouvement )
   {
      static int sensTheta = 1;
      static int sensPhi = 1;
      thetaCam += 0.3 * sensTheta;
      phiCam += 0.5 * sensPhi;
      //if ( thetaCam <= 0. || thetaCam >= 360.0 ) sensTheta = -sensTheta;
      if ( phiCam < -90.0 || phiCam > 90.0 ) sensPhi = -sensPhi;

      static int sensAngle = 1;
      LightSource[0].spotAngle += sensAngle * 0.3;
      if ( LightSource[0].spotAngle < 5.0 ) sensAngle = -sensAngle;
      if ( LightSource[0].spotAngle > 60.0 ) sensAngle = -sensAngle;

      // De temps à autre, alterner entre le modèle d'illumination: Lambert, Gouraud, Phong
      static float type = 0;
      type += 0.005;
      varsUnif.typeIllumination = fmod(type,3);
   }

   verifierAngles();
}


void updatePackets()
{
    // Compute wave packets
    packets->AdvectWavePackets(INIT_WAVE_SPEED);

    // TODO Setup wide projection for InitiateWaveField (RasterizeWaveMeshPosition)

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
            packetChunk += 3; /* The last 3 elements aren't needed */
            displayedPackets++;
            if (packetChunk >= PACKET_GPU_BUFFER_SIZE * 3 * 4) {
                glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
                /* TODO Use Buffer mapping for better performance */
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(packetData), packetData);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                //displayPacketOutlined(packetChunk / 12);
                /* TODO EvaluatePackets(packetChunk) */
                addPacketDisplacement(packetChunk / 12);
                packetChunk = 0;
            }
        }
    }
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
        packetChunk += 3; /* The last 3 elements aren't needed */
        displayedPackets++;
        if (packetChunk >= PACKET_GPU_BUFFER_SIZE * 3 * 4) {
            glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(packetData), packetData);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            //displayPacketOutlined();
            /* TODO EvaluatePackets(packetChunk) */
            addPacketDisplacement(packetChunk / 12);
            packetChunk = 0;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(packetData), packetData);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //displayPacketOutlined(packetChunk / 12);
    /* TODO EvaluatePackets(packetChunk) */
    addPacketDisplacement(packetChunk / 12);
    /* TODO DisplayScene */
    /* TODO Get camera center */
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
      locTexRWMP = getloc( progRasterizeWaveMeshPosition, "tex" , LocUniform);
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
      locTexAPD = getloc( progAddPacketDisplacement, "tex" , LocUniform);
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
      locTexDMM = getloc( progDisplayMicroMesh, "tex" , LocUniform);
   }

   // Load DisplayTerrain shader
   {
      const GLchar *shaders[3] = {"displayTerrain.vert", NULL, "displayTerrain.frag"};
      progDisplayTerrain = createShader(shaders);
      locVertexDT = getloc( progDisplayTerrain, "Vertex" , LocAttrib);
      locTexCoordDT = getloc( progDisplayTerrain, "TexCoord" , LocAttrib);
      locmatrModelDT = getloc( progDisplayTerrain, "matrModel" , LocUniform);
      locmatrVisuDT = getloc( progDisplayTerrain, "matrVisu" , LocUniform);
      locmatrProjDT = getloc( progDisplayTerrain, "matrProj" , LocUniform);
      locTexDT = getloc( progDisplayTerrain, "tex" , LocUniform);
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
    glVertexAttribPointer(locAtt1APD, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)4);
    glVertexAttribPointer(locAtt2APD, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)8);
    glEnableVertexAttribArray(locPosAPD);
    glEnableVertexAttribArray(locAtt1APD);
    glEnableVertexAttribArray(locAtt2APD);

    glBindVertexArray(vao[DPO_SHADER]);
    glBindBuffer(GL_ARRAY_BUFFER, vboPacket);
    /* No need to initialize the buffer a second time */
    glVertexAttribPointer(locPosDPO, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)0);
    glVertexAttribPointer(locAtt1DPO, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)4);
    glVertexAttribPointer(locAtt2DPO, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)8);
    glEnableVertexAttribArray(locPosDPO);
    glEnableVertexAttribArray(locAtt1DPO);
    glEnableVertexAttribArray(locAtt2DPO);

    glBindVertexArray(0);
}


// initialisation d'openGL
void initialiser()
{
   // donner l'orientation du modèle
   thetaCam = 0.0;
   phiCam = 0.0;
   distCam = 90.0;

   // Create FBOs
   posFBO = new FBO();
   heightFBO = new FBO();
   aaFBO = new FBO();

   // Create Packets
   packets = new Packets(packetBudget);

   // couleur de l'arrière-plan
   glClearColor( 0.4, 0.2, 0.0, 1.0 );

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

   // créer quelques autres formes
   sphere = new FormeSphere( 1.0, 32, 32 );
   sphereLumi = new FormeSphere( 0.5, 10, 10 );
   theiere = new FormeTheiere( );
   tore = new FormeTore( 0.4, 0.8, 32, 32 );
   cylindre = new FormeCylindre( 0.3, 0.3, 3.0, 32, 32 );
   cone = new FormeCylindre( 0.0, 0.5, 3.0, 32, 32 );

   // Update display mesh and FBOs
   // redimensionner();
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

void drawQuad()
{
    glUseProgram(progRasterizeWaveMeshPosition);
    glUniformMatrix4fv( locmatrProjRWMP, 1, GL_FALSE, matrProj );
    glUniformMatrix4fv( locmatrVisuRWMP, 1, GL_FALSE, matrVisu );
    glUniformMatrix4fv( locmatrModelRWMP, 1, GL_FALSE, matrModel );
    glBindVertexArray(vao[RWMP_SHADER]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
    glBindVertexArray(vao[APD_SHADER]);
    glUniformMatrix4fv(locmatrModelAPD, 1, GL_FALSE, matrModel);
    glUniformMatrix4fv(locmatrVisuAPD, 1, GL_FALSE, matrVisu);
    glUniformMatrix4fv(locmatrProjAPD, 1, GL_FALSE, matrProj);
    glDrawArrays(GL_POINTS, 0, count);
    glBindVertexArray(0);
    glUseProgram(0);
}


void afficherModele()
{
   // Dessiner le modèle
   matrModel.PushMatrix(); {

      // appliquer les rotations
      matrModel.Rotate( phiCam, -1.0, 0.0, 0.0 );
      matrModel.Rotate( thetaCam, 0.0, -1.0, 0.0 );

      // mise à l'échelle
      matrModel.Scale( 5.0, 5.0, 5.0 );

      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      updatePackets();
   } matrModel.PopMatrix();
}

void afficherLumiere()
{
   // Dessiner la lumiere

   // tracer une ligne vers la source lumineuse
   const GLfloat fact = 5.;
   GLfloat coords[] =
   {
      LightSource[0].position.x                                    , LightSource[0].position.y                                    , LightSource[0].position.z,
      LightSource[0].position.x+LightSource[0].spotDirection.x/fact, LightSource[0].position.y+LightSource[0].spotDirection.y/fact, LightSource[0].position.z+LightSource[0].spotDirection.z/fact
   };
   glLineWidth( 3.0 );
   glVertexAttrib3f( locColorBase, 1.0, 1.0, 0.5 ); // jaune
   glBindVertexArray( vao[1] );
   matrModel.PushMatrix(); {
      glBindBuffer( GL_ARRAY_BUFFER, vbo[4] );
      glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(coords), coords );
      glDrawArrays( GL_LINES, 0, 2 );
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
   glBindVertexArray( 0 );
   glLineWidth( 1.0 );

   // tracer la source lumineuse
   matrModel.PushMatrix(); {
      matrModel.Translate( LightSource[0].position.x, LightSource[0].position.y, LightSource[0].position.z );
      glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      sphereLumi->afficher();
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
}

// fonction d'affichage
void FenetreTP::afficherScene()
{
   // effacer l'ecran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   if ( enPerspective )
   {
      matrProj.Perspective( 35.0, (GLdouble)largeur_ / (GLdouble)hauteur_,
                            0.1, 300.0 );
   }
   else
   {
      const GLfloat d = 8.0;
      if ( largeur_ <= hauteur_ )
      {
         matrProj.Ortho( -d, d,
                         -d*(GLdouble)hauteur_ / (GLdouble)largeur_,
                         d*(GLdouble)hauteur_ / (GLdouble)largeur_,
                         0.1, 300.0 );
      }
      else
      {
         matrProj.Ortho( -d*(GLdouble)largeur_ / (GLdouble)hauteur_,
                         d*(GLdouble)largeur_ / (GLdouble)hauteur_,
                         -d, d,
                         0.1, 300.0 );
      }
   }
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   matrVisu.LookAt( 0.0, 3.0, distCam,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0 );
   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes( 8.0 );

   // dessiner la scène
   //glActiveTexture( GL_TEXTURE0 ); // activer la texture '0' (valeur de défaut)
   glUniform1i( loclaTexture, 0 ); // '0' => utilisation de GL_TEXTURE0

   afficherModele();
}

// fonction de redimensionnement de la fenêtre graphique
void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
   std::cout << "Resizing to " << w << "×" << h << std::endl;
   /* FIXME Is this function called on program start ? */
   glViewport( 0, 0, w, h );
   /* TODO Create/resize display mesh */
   posFBO->Liberer();
   posFBO->Init(WAVETEX_WIDTH_FACTOR * w, WAVETEX_HEIGHT_FACTOR * h);
   heightFBO->Liberer();
   heightFBO->Init(WAVETEX_WIDTH_FACTOR * w, WAVETEX_HEIGHT_FACTOR * h);
   aaFBO->Liberer();
   aaFBO->Init(AA_OVERSAMPLE_FACTOR * w, AA_OVERSAMPLE_FACTOR * h);
}

static void echoEtats( )
{
   static std::string illuminationStr[] = { "0:Lambert", "1:Gouraud", "2:Phong" };
   static std::string reflexionStr[] = { "0:Phong", "1:Blinn" };
   static std::string spotStr[] = { "0:OpenGL", "1:Direct3D" };
   std::cout << " modèle d'illumination= " << illuminationStr[varsUnif.typeIllumination]
             << ", refléxion spéculaire= " << reflexionStr[varsUnif.utiliseBlinn]
             << ", spot= " << spotStr[varsUnif.utiliseDirect]
             << std::endl;
}

// fonction de gestion du clavier
void FenetreTP::clavier( TP_touche touche )
{
   // traitement des touches q et echap
   switch ( touche )
   {
   case TP_ECHAP:
   case TP_q: // Quitter l'application
      quit();
      break;

   case TP_x: // Activer/désactiver l'affichage des axes
      afficheAxes = !afficheAxes;
      std::cout << "// Affichage des axes ? " << ( afficheAxes ? "OUI" : "NON" ) << std::endl;
      break;

   case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
      chargerNuanceurs();
      std::cout << "// Recharger nuanceurs" << std::endl;
      break;

   case TP_p: // Permuter la projection: perspective ou orthogonale
      enPerspective = !enPerspective;
      break;

   case TP_i: // Alterner entre le modèle d'illumination: Lambert, Gouraud, Phong
      if ( ++varsUnif.typeIllumination > 2 ) varsUnif.typeIllumination = 0;
      echoEtats( );
      break;

   case TP_r: // Alterner entre le modèle de réflexion spéculaire: Phong, Blinn
       // Send one wave front
       packets->CreateCircularWavefront(0.0, 0.0, 2.0, 0.2, 1.0, 10000);
       printf("Sending a circular wavefront...\n");
      break;

   case TP_s: // Alterner entre le modèle de spot: OpenGL, Direct3D
      varsUnif.utiliseDirect = !varsUnif.utiliseDirect;
      echoEtats( );
      break;

   //case TP_l: // Alterner entre une caméra locale à la scène ou distante (localViewer)
   //   LightModel.localViewer = !LightModel.localViewer;
   //   std::cout << " localViewer=" << LightModel.localViewer << std::endl;
   //   break;

   case TP_a: // Incrémenter l'angle du cône du spot
   case TP_EGAL:
   case TP_PLUS:
      LightSource[0].spotAngle += 2.0;
      if ( LightSource[0].spotAngle > 90.0 ) LightSource[0].spotAngle = 90.0;
      std::cout <<  " spotAngle=" << LightSource[0].spotAngle << std::endl;
      break;
   case TP_z: // Décrémenter l'angle du cône du spot
   case TP_MOINS:
   case TP_SOULIGNE:
      LightSource[0].spotAngle -= 2.0;
      if ( LightSource[0].spotAngle < 0.0 ) LightSource[0].spotAngle = 0.0;
      std::cout <<  " spotAngle=" << LightSource[0].spotAngle << std::endl;
      break;

   case TP_d: // Incrémenter l'exposant du spot
   case TP_BARREOBLIQUE:
      LightSource[0].spotExposant += 0.3;
      if ( LightSource[0].spotExposant > 89.0 ) LightSource[0].spotExposant = 89.0;
      std::cout <<  " spotExposant=" << LightSource[0].spotExposant << std::endl;
      break;
   case TP_e: // Décrémenter l'exposant du spot
   case TP_POINT:
      LightSource[0].spotExposant -= 0.3;
      if ( LightSource[0].spotExposant < 0.0 ) LightSource[0].spotExposant = 0.0;
      std::cout <<  " spotExposant=" << LightSource[0].spotExposant << std::endl;
      break;

   case TP_j: // Incrémenter le coefficient de brillance
   case TP_CROCHETDROIT:
      FrontMaterial.shininess *= 1.1;
      std::cout << " FrontMaterial.shininess=" << FrontMaterial.shininess << std::endl;
      break;
   case TP_u: // Décrémenter le coefficient de brillance
   case TP_CROCHETGAUCHE:
      FrontMaterial.shininess /= 1.1; if ( FrontMaterial.shininess < 0.0 ) FrontMaterial.shininess = 0.0;
      std::cout << " FrontMaterial.shininess=" << FrontMaterial.shininess << std::endl;
      break;

   case TP_DROITE:
      LightSource[0].position.x += 0.3;
      break;
   case TP_GAUCHE:
      LightSource[0].position.x -= 0.3;
      break;
   case TP_BAS:
      LightSource[0].position.y += 0.3;
      break;
   case TP_HAUT:
      LightSource[0].position.y -= 0.3;
      break;

   case TP_FIN:
      LightSource[0].spotDirection.x += 0.6;
      break;
   case TP_DEBUT:
      LightSource[0].spotDirection.x -= 0.6;
      break;
   case TP_PAGEPREC:
      LightSource[0].spotDirection.y += 0.6;
      break;
   case TP_PAGESUIV:
      LightSource[0].spotDirection.y -= 0.6;
      break;

   case TP_m: // Choisir le modèle affiché: cube, tore, sphère, théière, cylindre, cône
      if ( ++modele > 6 ) modele = 1;
      std::cout << " modele=" << modele << std::endl;
      break;

   case TP_0:
      thetaCam = 0.0; phiCam = 0.0; distCam = 30.0; // placer les choses afin d'avoir une belle vue
      break;

   case TP_t: // Choisir la texture utilisée: aucune, dé, échiquier
      varsUnif.texnumero++;
      if ( varsUnif.texnumero > 2 ) varsUnif.texnumero = 0;
      std::cout << " varsUnif.texnumero=" << varsUnif.texnumero << std::endl;
      break;

   // case TP_c: // Changer l'affichage de l'objet texturé avec couleurs ou sans couleur
   //    varsUnif.utiliseCouleur = !varsUnif.utiliseCouleur;
   //    std::cout << " utiliseCouleur=" << varsUnif.utiliseCouleur << std::endl;
   //    break;

   case TP_o: // Changer l'affichage des texels noirs (noir, mi-coloré, transparent)
      varsUnif.afficheTexelNoir++;
      if ( varsUnif.afficheTexelNoir > 2 ) varsUnif.afficheTexelNoir = 0;
      std::cout << " afficheTexelNoir=" << varsUnif.afficheTexelNoir << std::endl;
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
      break;

   case TP_n: // Utiliser ou non les normales calculées comme couleur (pour le débogage)
      varsUnif.afficheNormales = !varsUnif.afficheNormales;
      break;

   case TP_ESPACE: // Permuter la rotation automatique du modèle
      enmouvement = !enmouvement;
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

void FenetreTP::sourisWheel( int x, int y ) // Changer la taille du spot
{
   const int sens = +1;
   LightSource[0].spotAngle += sens*y;
   if ( LightSource[0].spotAngle > 90.0 ) LightSource[0].spotAngle = 90.0;
   if ( LightSource[0].spotAngle < 0.0 ) LightSource[0].spotAngle = 0.0;
   std::cout <<  " spotAngle=" << LightSource[0].spotAngle << std::endl;
}

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
         thetaCam -= dx / 3.0;
         phiCam   -= dy / 3.0;
         break;
      case deplaceSpotDirection:
         LightSource[0].spotDirection.x += 0.06 * dx;
         LightSource[0].spotDirection.y -= 0.06 * dy;
         // std::cout << " LightSource[0].spotDirection=" << glm::to_string(LightSource[0].spotDirection) << std::endl;
         break;
      case deplaceSpotPosition:
         LightSource[0].position.x += 0.03 * dx;
         LightSource[0].position.y -= 0.03 * dy;
         // std::cout << " LightSource[0].position=" << glm::to_string(LightSource[0].position) << std::endl;
         //glm::vec3 ecranPos( x, hauteur_-y, ecranLumi[2] );
         //LightSource[0].position = glm::vec4(glm::unProject( ecranPos, VM, P, cloture ), 1.0);
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
   FenetreTP fenetre( "INF2705 TP" );

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
