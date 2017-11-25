#version 410

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position;
   vec3 spotDirection;
   float spotExponent;
   float spotCutoff;            // ([0.0,90.0] ou 180.0)
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;
} LightSource[1];

// Définition des paramètres des matériaux
layout (std140) uniform MaterialParameters
{
   vec4 emission;
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   float shininess;
} FrontMaterial;

// Définition des paramètres globaux du modèle de lumière
layout (std140) uniform LightModelParameters
{
   vec4 ambient;       // couleur ambiante
   bool localViewer;   // observateur local ou à l'infini?
   bool twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel;

layout (std140) uniform varsUnif
{
   // partie 1: illumination
   int typeIllumination;     // 0:Lambert, 1:Gouraud, 2:Phong
   bool utiliseBlinn;        // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
   bool utiliseDirect;       // indique si on utilise un spot style Direct3D ou OpenGL
   bool afficheNormales;     // indique si on utilise les normales comme couleurs (utile pour le débogage)
   // partie 3: texture
   int texnumero;            // numéro de la texture appliquée
   bool utiliseCouleur;      // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
   int afficheTexelNoir;     // un texel noir doit-il être affiché 0:noir, 1:mi-coloré, 2:transparent?
};

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;
uniform mat3 matrNormale;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec4 Vertex;
layout(location=2) in vec3 Normal;
layout(location=3) in vec4 Color;
layout(location=8) in vec4 TexCoord;

out Attribs {
   vec3 lumiDir, spotDir;
   vec3 normale, obsVec;
   vec2 texCoord;
   vec4 couleur;
} AttribsOut;

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
   vec4 coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;

   // calcul de la composante ambiante
   coul += FrontMaterial.ambient * LightSource[0].ambient;

   // calcul de l'éclairage seulement si le produit scalaire est positif
   float NdotL = max( 0.0, dot( N, L ) );
   if ( NdotL > 0.0 )
   {
      // calcul de la composante diffuse
      //coul += ( utiliseCouleur ? FrontMaterial.diffuse : vec4(1.0) ) * LightSource[0].diffuse * NdotL;
      coul += FrontMaterial.diffuse * LightSource[0].diffuse * NdotL;

      // calcul de la composante spéculaire (Blinn ou Phong)
      float NdotHV = max( 0.0, ( utiliseBlinn ) ? dot( normalize( L + O ), N ) : dot( reflect( -L, N ), O ) );
      coul += FrontMaterial.specular * LightSource[0].specular * ( ( NdotHV == 0.0 ) ? 0.0 : pow( NdotHV, FrontMaterial.shininess ) );
   }
   return( coul );
}

void main( void )
{
   // transformation standard du sommet, ** sans la projection **
   gl_Position = matrVisu * matrModel * Vertex;

   // calculer la normale qui sera interpolée pour le nuanceur de fragment
   AttribsOut.normale = matrNormale * Normal;

   // calculer la position du sommet (dans le repère de la caméra)
   vec3 pos = vec3( matrVisu * matrModel * Vertex );

   // vecteur de la direction de la lumière (dans le repère de la caméra)
   AttribsOut.lumiDir = vec3( ( matrVisu * LightSource[0].position ).xyz - pos );

   // vecteur de la direction vers l'observateur (dans le repère de la caméra)
   AttribsOut.obsVec = ( LightModel.localViewer ?
                         normalize(-pos) :        // =(0-pos) un vecteur qui pointe vers le (0,0,0), c'est-à-dire vers la caméra
                         vec3( 0.0, 0.0, 1.0 ) ); // on considère que l'observateur (la caméra) est à l'infini dans la direction (0,0,1)
   // vecteur de la direction du spot (en tenant compte seulement des rotations de la caméra)
   AttribsOut.spotDir = inverse(mat3(matrVisu)) * -LightSource[0].spotDirection;
   // On accepte aussi: (si on suppose que .spotDirection est déjà dans le repère de la caméra)
   //AttribsOut.spotDir = -LightSource[0].spotDirection;
   // On accepte aussi: (car matrVisu a seulement une translation et pas de rotation => "mat3(matrVisu) == I" )
   //AttribsOut.spotDir = -LightSource[0].spotDirection;
   // On accepte aussi: (car c'était le calcul qui était dans la solution précédente présentée dans le lab!)
   //AttribsOut.spotDir = -( matrVisu * vec4(LightSource[0].spotDirection,1.0) ).xyz;

   // si illumination est 1:Gouraud, calculer la réflexion ici, sinon ne rien faire de plus
   if ( typeIllumination == 1 )
   {
      vec3 L = normalize( AttribsOut.lumiDir ); // calcul du vecteur de la surface vers la source lumineuse
      vec3 N = normalize( AttribsOut.normale ); // vecteur normal
      vec3 O = normalize( AttribsOut.obsVec );  // position de l'observateur
      AttribsOut.couleur = calculerReflexion( L, N, O );
   }
   //else
   //   couleur = vec4(0.0); // inutile

   // assigner les coordonnées de texture
   AttribsOut.texCoord = TexCoord.st;
}
