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

uniform sampler2D laTexture;

/////////////////////////////////////////////////////////////////

in Attribs {
   vec3 lumiDir, spotDir;
   vec3 normale, obsVec;
   vec2 texCoord;
   vec4 couleur;
} AttribsIn;

out vec4 FragColor;

float calculerSpot( in vec3 spotDir, in vec3 L )
{
   float spotFacteur;
   float spotDot = dot( L, normalize( spotDir ) );
   if ( utiliseDirect ) // modèle Direct3D
   {
      float cosAngleInterne = cos(radians(LightSource[0].spotCutoff));
      float exposant = 1.01 + LightSource[0].spotExponent / 2.0;
      float cosAngleExterne = pow( cos(radians(LightSource[0].spotCutoff)), exposant );
      // calculer le facteur spot avec la fonction smoothstep()
      spotFacteur = smoothstep( cosAngleExterne, cosAngleInterne, spotDot );
   }
   else // modèle OpenGL
   {
      spotFacteur = ( spotDot > cos(radians(LightSource[0].spotCutoff)) ) ? pow( spotDot, LightSource[0].spotExponent ) : 0.0;
   }
   return( spotFacteur );
}

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
   vec3 L = normalize( AttribsIn.lumiDir ); // vecteur vers la source lumineuse
   vec3 N = normalize( AttribsIn.normale ); // vecteur normal
   //vec3 N = normalize( gl_FrontFacing ? AttribsIn.normale : -AttribsIn.normale );
   vec3 O = normalize( AttribsIn.obsVec );  // position de l'observateur

   // calculer la réflexion:
   //   si illumination de 1:Gouraud, prendre la couleur interpolée qui a été reçue
   //   si illumination de 2:Phong, le faire!
   //   si illumination de 0:Lambert, faire comme Phong, même si les normales sont les mêmes pour tous les fragments
   vec4 coul = ( typeIllumination == 1 ) ? AttribsIn.couleur : calculerReflexion( L, N, O );

   // calculer l'influence du spot
   float spotFacteur = calculerSpot( AttribsIn.spotDir, L );
   coul *= spotFacteur;
   //if ( spotFacteur <= 0.0 ) discard; // pour éliminer tout ce qui n'est pas dans le cône
   // calcul de la composante ambiante
   //coul += FrontMaterial.ambient * LightSource[0].ambient;

   // appliquer la texture s'il y a lieu
   if ( texnumero != 0 )
   {
      vec4 couleurTexture = texture( laTexture, AttribsIn.texCoord );
      // comment afficher un texel noir?
      if ( couleurTexture.r < 0.1 && couleurTexture.g < 0.1 && couleurTexture.b < 0.1 &&
           spotFacteur > 0.0 )
         if ( afficheTexelNoir == 1 )
            couleurTexture = coul / 2.0;
         else if ( afficheTexelNoir == 2 )
            discard;
      coul *= couleurTexture;
   }

   // assigner la couleur finale
   FragColor = clamp( coul, 0.0, 1.0 );

   if ( afficheNormales ) FragColor = vec4(N,1.0);
}
