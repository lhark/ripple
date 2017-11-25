#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;
uniform mat3 matrNormale;

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

in Attribs {
   vec3 lumiDir, spotDir;
   vec3 normale, obsVec;
   vec2 texCoord;
   vec4 couleur;
} AttribsIn[];

out Attribs {
   vec3 lumiDir, spotDir;
   vec3 normale, obsVec;
   vec2 texCoord;
   vec4 couleur;
} AttribsOut;

void main()
{
   // si illumination est Lambert, calculer une nouvelle normale
   vec3 n = vec3(0.0);
   if ( typeIllumination == 0 )
   {
      vec3 p0 = gl_in[0].gl_Position.xyz;
      vec3 p1 = gl_in[1].gl_Position.xyz;
      vec3 p2 = gl_in[2].gl_Position.xyz;
      n = cross( p1-p0, p2-p0 ); // cette nouvelle normale est déjà dans le repère de la caméra
                                 // il n'est pas nécessaire de la multiplier par matrNormale
   }
   // ou faire une moyenne, MAIS CE N'EST PAS CE QU'ON VEUT!
   // if ( typeIllumination == 0 )
   // {
   //    // calculer le centre
   //    for ( int i = 0 ; i < gl_in.length() ; ++i )
   //    {
   //       n += AttribsIn[i].normale;
   //    }
   //    n /= gl_in.length();
   // }

   // émettre les sommets
   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      gl_Position = matrProj * gl_in[i].gl_Position; // on termine la transformation débutée dans le nuanceur de sommets
      AttribsOut.lumiDir = AttribsIn[i].lumiDir;
      AttribsOut.spotDir = AttribsIn[i].spotDir;
      AttribsOut.normale = ( typeIllumination == 0 ) ? n : AttribsIn[i].normale;
      AttribsOut.obsVec = AttribsIn[i].obsVec;
      AttribsOut.texCoord = AttribsIn[i].texCoord;
      AttribsOut.couleur = AttribsIn[i].couleur;
      EmitVertex();
   }
}
