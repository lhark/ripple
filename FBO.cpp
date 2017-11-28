///////////////////////////////////////////////////////////////////////////////
///  @file FBO.cpp
///  @author  Olivier Dionne
///  @brief   Définit la classe FBO implémentant un Frame Buffer Object simple
///           pour openGL
///  @date    2008-10-19
///  @version 1.0
///
///////////////////////////////////////////////////////////////////////////////
#include "FBO.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
///  public overloaded constructor  FBO \n
///
///  Crée un FBO très simple où tous les membres privés sont à 0.
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
FBO::FBO() :
m_FBO( 0 ),
m_Texture( 0 ),
m_InternalFormat( 0 ),
m_Target( GL_TEXTURE_2D ),
m_TextureW( 0 ),
m_TextureH( 0 )
{
}

///////////////////////////////////////////////////////////////////////////////
///  public destructor  ~FBO \n
///
///  Détruit un objet FBO
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
FBO::~FBO()
{
   Liberer();
}


///////////////////////////////////////////////////////////////////////////////
///  public  Init \n
///
///  Cette méthode initialise le FBO en créant les noms de texture à l'interne.
///  Pour une mise à jour du FBO à chaque image, cette fonction NE DEVRAIT PAS
///  être appelée constamment, parce qu'elle consomme de la mémoire et est
///  relativement lente. On conseille donc d'appeler Init() une seule fois au début
///  de l'application, puis simplement d'utiliser la paire CommencerCapture() et
///  TerminerCapture() afin de mettre à jour le contenu interne du FBO en tout temps.
///  Évidemment, parce qu'il est important que la taille du FBO soit la même que le
///  viewport courant, si la taille du viewport change en cours d'exécution, il est
///  impératif d'appeler Init() à nouveau (avec les nouvelles valeurs de w et de h), mais
///  en ayant pris soin au préalable de LIBERER la mémoire du FBO.
///
///  @param [in]       w int   La largeur du viewport / de la texture interne
///  @param [in]       h int   La hauteur du viewport / de la texture interne
///  @param [in]       format int   Le format interne de la texture du FBO (communément GL_RGB)
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void FBO::Init( int w, int h )
{
   // Dimensions
   m_TextureW = w;
   m_TextureH = h;

   // TODO: Remplir la fonction d'initialisation d'un FBO:

   // Créer et lier un nouveau frame buffer avec l'ID m_fbo:
   glGenFramebuffers(1, &m_FBO);
   glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

   // Créer une texture RGB pour les couleurs avec L'ID m_Texture:
   // Pour échantillionner plus tard des valeurs exactes
   // on veut des filtres de mignification et magnification de tpe NEAREST!
   glGenTextures(1, &m_Texture);
   glBindTexture(m_Target, m_Texture);
   glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri( m_Target, GL_TEXTURE_WRAP_S, GL_CLAMP );
   glTexParameteri( m_Target, GL_TEXTURE_WRAP_T, GL_CLAMP );
   glTexImage2D(m_Target, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

   // Créer une texture de profondeurs pour les couleurs avec L'ID m_Profondeur:
   glGenTextures(1, &m_Profondeur);
   glBindTexture(m_Target, m_Profondeur);
   glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri( m_Target, GL_TEXTURE_WRAP_S, GL_CLAMP );
   glTexParameteri( m_Target, GL_TEXTURE_WRAP_T, GL_CLAMP );
   glTexImage2D(m_Target, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);


   // Attacher nos deux textures au frame buffer à des fin d'affichage (DRAW):
   glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_Target, m_Texture, 0);
   glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_Target, m_Profondeur, 0);

   // Vérification des erreurs FBO
   // Nous vous fournissons cette vérification d'erreurs
   // pour que vous arriviez plus aisément à déboguer votre code.
   GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // On vérifie les erreurs à la suite de la création du FBO
   switch( status )
   {
   case GL_FRAMEBUFFER_COMPLETE_EXT:
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT" << endl;
      break;
   case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      cerr << "GL_FRAMEBUFFER_UNSUPPORTED_EXT" << endl;
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT" << endl;
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT" << endl;
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT" << endl;
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT" << endl;
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT" << endl;
      break;
   default:
      cerr << "ERREUR INCONNUE" << endl;
   }
}

///////////////////////////////////////////////////////////////////////////////
///  public  Liberer \n
///
///  Cette fonction libère l'espace mémoire interne du FBO
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void FBO::Liberer()
{
   if ( m_Texture )
   {
      glDeleteTextures( 1, &m_Texture );
      m_Texture = 0;
   }

   if ( m_FBO )
   {
      glDeleteFramebuffers( 1, &m_FBO );
      m_FBO = 0;
   }
   if(m_Profondeur)
   {
	   glDeleteTextures( 1, &m_Profondeur );
      m_Profondeur = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////
///  public  CommencerCapture \n
///
///  Cette fonction débute la définition du contenu du FBO. Son utilisation est
///  très simple et intuitive. Une fois le FBO construit et initialiser avec new ()
///  et Init(), on n'a qu'à insérer les commandes openGL produisant le rendu externe
///  voulu (qui sera enregistré dans le FBO) entre les commandes CommencerCapture() et
///  TerminerCapture();
///
///  @return Aucune
///
///  @author Olivier Dionne, Frédéric Plourde
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void FBO::CommencerCapture()
{
    // TODO:
	// Activer l'utilisation du FBO
	// Attention à la résolution avec laquelle on veut afficher!
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,m_TextureW, m_TextureH);
}

///////////////////////////////////////////////////////////////////////////////
///  public  TerminerCapture \n
///
///  Cette fonction termine la définition du contenu du FBO. Son utilisation est
///  très simple et intuitive. Une fois le FBO construit et initialiser avec new ()
///  et Init(), on n'a qu'à insérer les commandes openGL produisant le rendu externe
///  voulu (qui sera enregistré dans le FBO) entre les commandes CommencerCapture() et
///  TerminerCapture();
///
///  @return Aucune
///
///  @author Olivier Dionne
///  @date   2008-10-19
///
///////////////////////////////////////////////////////////////////////////////
void FBO::TerminerCapture()
{
	// TODO:
	// Remettre OpenGL dans l'état par défaut
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();
}
