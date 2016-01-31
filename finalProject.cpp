//
// CameraExample.cpp
//
// Example program to show how to use Chris Root's OpenGL Camera Class
// 
// Christopher Root, 2006
// Minor Modifications by Donald House, 2009
// Minor Modifications by Yujie Shu, 2012
//
#include "Camera.h"
#include "finalProject.h"
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "tga.h"

#include <fstream>
#include <sstream>

#define TimerDelay 16


int framecount =0;

static char *ParamFilename = NULL;

int WIDTH = 1280;
//int WIDTH = 1920;
int HEIGHT = 960;
//int HEIGHT = 1080;
//int WIDTH = 800;
//int HEIGHT = 600;

float Kspring = 60;
float Kdamping = 5;

float KspringMax = 200;

//float RADIUS = 1.5;
float EPSILON = 0.05;

int persp_win;

Camera *camera;

void TimerCallback(int);


bool showGrid = true;

SpringVertex trampoline1[20][20],trampoline2[20][20];

bool collisionMatrix1[20][20];
bool collisionMatrix2[20][20];

///RigidBody Ball;
RigidBody RB;

Vector3d gravity(0,-9.8,0);

float YFloor = -12;
float FloorEpsilon = 0.05;

float Restitution = 0.9;
float LeastValue = 0.01;

float Friction = 0.15;

float FrictionFloor = 0.1;

StateVector SV;
//StateVectorBall SVBall;

float xStart1 = -13.5 ;
float yStart1 = 10.0 ;
float zStart1 = 0.0 ;

float xStart2 = 14.5 ;
float yStart2 = -1.0 ;
float zStart2 = -9.5 ;

float horizontalRestDistance = 0.5;
float verticalRestDistance = 0.5;
float diagonalRestDistance = 0.707;


GLuint Texture[128];		// Handles to our textures

#define PACKED __attribute__((packed))

struct TGAHeader

{

	unsigned char  identsize		PACKED;   // size of ID field that follows 18 uint8 header (0 usually)

	unsigned char  colourmaptype	PACKED;   // type of colour map 0=none, 1=has palette

	unsigned char  imagetype		PACKED;   // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed



	unsigned short colourmapstart	PACKED;   // first colour map entry in palette

	unsigned short colourmaplength	PACKED;   // number of colours in palette

	unsigned char  colourmapbits	PACKED;   // number of bits per palette entry 15,16,24,32



	unsigned short xstart			PACKED;   // image x origin

	unsigned short ystart			PACKED;   // image y origin

	unsigned short width			PACKED;   // image width in pixels

	unsigned short height			PACKED;   // image height in pixels

	unsigned char  bits				PACKED;   // image bits per pixel 8,16,24,32

	unsigned char  descriptor		PACKED;   // image descriptor bits (vh flip bits)



	inline bool IsFlippedHorizontal() const
	{

		return (descriptor & 0x10) != 0;

	}



	inline bool IsFlippedVertical() const
	{

		return (descriptor & 0x20) != 0;

	}



	// pixel data follows header  

};

#undef PACKED

void ScreenShot( std::string fileName, unsigned int width, unsigned int height, bool includeAlpha = false )
{
	unsigned int pixelSize = 3;
	unsigned int pixelSizeBits = 24;
	GLenum pixelFormat = GL_BGR_EXT;

	if(includeAlpha)
	{
		pixelSize = sizeof(unsigned int);
		pixelSizeBits = 32;
		pixelFormat = GL_BGRA_EXT;
	}

	char* pBuffer = new char[pixelSize*width*height ];

	glReadPixels( 0,0,width,height,pixelFormat,GL_UNSIGNED_BYTE,pBuffer );

	TGAHeader tgah;
	memset( &tgah,0,sizeof(TGAHeader) );

	tgah.bits = pixelSizeBits;
	tgah.height = height;
	tgah.width = width;
	tgah.imagetype = 2;

	std::ofstream ofile( fileName.c_str(), std::ios_base::binary );

	ofile.write( (char*)&tgah, sizeof(tgah) );
	ofile.write( pBuffer, pixelSize*width*height );

	ofile.close();

	delete [] pBuffer;
}

string int_to_string(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}


// Load a TGA texture

bool LoadTexture(char *TexName, GLuint TexHandle) {

	TGAImg Img;        // Image loader



	// Load our Texture

	if(Img.Load(TexName)!=IMG_OK)

		return false;

	

	// Set our Tex handle as current

	glBindTexture(GL_TEXTURE_2D,TexHandle);



	// Create the texture

	if(Img.GetBPP()==24)

		glTexImage2D(GL_TEXTURE_2D,0,3,Img.GetWidth(),Img.GetHeight(),0,GL_RGB,GL_UNSIGNED_BYTE,Img.GetImg());

	else if(Img.GetBPP()==32)

		glTexImage2D(GL_TEXTURE_2D,0,4,Img.GetWidth(),Img.GetHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,Img.GetImg());

	else

		return false;



	// Specify filtering and edge actions

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);



	return true;

}




void do_lights() {

	//float light0_ambient[] = {0.2,0.2,0.2,0.0};
	float light0_ambient[] = {0.7,0.7,0.7,0.0};

	float light0_diffuse[] = {5.0,20.0,5.0,0.0};

	float light0_specular[] = {2.25,20.25,2.25,0.0};

	float light0_position[] = {0,10.0,30.0,1.0};

	float light0_direction[] = {0,0,10,1.0};

	

	//float light1_ambient[] = {0.2,0.2,0.2,0.0};
	//float light1_ambient[] = {0.8,0.8,0.8,0.0};
	float light1_ambient[] = {0.7,0.7,0.7,0.0};
	
	
	float light1_diffuse[] = {5.0,5.0,25.0,0.0};

	float light1_specular[] = {2.25,20.25,2.25,0.0};

	float light1_position[] = {0,0.0,-30.0,1.0};

	float light1_direction[] = {0,0,10,1.0};
	
	
		

	// set scene default ambient

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light0_ambient);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

	
// make specular correct for spots	

	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);

	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0);

	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);

	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.2);

	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01);

	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_direction);

	

	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);

	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1.0);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180.0);

	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0);

	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2);

	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01);

	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_direction);
	

	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);

	glEnable(GL_LIGHT1);
	
	
}




void init() {

	// set up camera
	// parameters are eye point, aim point, up vector
	//camera = new Camera(Vector3d(0, 50 , 50), Vector3d(0, 0, 0), Vector3d(0, 1, 0));
	camera = new Camera(Vector3d(0, 1.5 , 20), Vector3d(-1, 0, 10), Vector3d(0, 1, 0));
  
	glClearColor(0, 0, 0, 0);
	glShadeModel(GL_SMOOTH);
	// glDepthRange(0.0, 1.0);

	//glEnable(GL_BLEND);
  
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
  
	// glEnable (GL_LIGHTING);
    //glEnable (GL_LIGHT0);
  
  
	// Allocate all textures in one go

	glGenTextures(128,Texture);	

	do_lights();

	// Load the textures

	LoadTexture("Textures/court2.tga",Texture[0]);
	//LoadTexture("Textures/Nball.tga",Texture[1]);
	LoadTexture("Textures/football.tga",Texture[1]);
	//LoadTexture("Textures/tennisball.tga",Texture[1]);
	//LoadTexture("Textures/newtennisball.tga",Texture[1]);
	//LoadTexture("Textures/newbaseball.tga",Texture[1]);
	


	//glEnable(GL_TEXTURE_2D);
  
  
  
	diagonalRestDistance = sqrt(horizontalRestDistance*horizontalRestDistance + verticalRestDistance*verticalRestDistance);
  
	//initializing tramp 1
	for(int x= 0 ; x< 20 ; x++ )
	{			
		for(int z=0 ; z<20 ; z++ )
		{
				trampoline1[x][z].Position.x = xStart1  + (float(x)/2) ;
				
				trampoline1[x][z].Position.y = yStart1  - (float(x)/2) ;
				
				trampoline1[x][z].Position.z = zStart1  - (float(z)/2) ;	
		}		
	}
  
	//initializing tramp 2
	for(int x= 0 ; x< 20 ; x++ )
	{			
		for(int z=0 ; z<20 ; z++ )
		{
				trampoline2[x][z].Position.x = xStart2  - (float(x)/2) ;
				
				trampoline2[x][z].Position.y = yStart2  - (float(x)/2) ;
				
				trampoline2[x][z].Position.z = zStart2  + (float(z)/2) ;	
		}		
	}
  
	int ctr =0;
  
	//initialise statevector
  
	// position of tramp 1
	for(int x= 0 ; x< 20 ; x++ )
	{			
		for(int z=0 ; z<20 ; z++ )
		{
			SV.arr[ctr] = trampoline1[x][z].Position ;
			ctr++;
		}

	}

  
	ctr =0;
  
  
	// position of tramp 2
	for(int x= 0 ; x< 20 ; x++ )
	{			
		for(int z=0 ; z<20 ; z++ )
		{
			SV.arr[ctr+400] = trampoline2[x][z].Position ;
			ctr++;
		}

	}  
  
	ctr =0;
  
	// all velocities
	for(int x= 0 ; x< 20 ; x++ )
	{			
		for(int z=0 ; z<20 ; z++ )
		{
			
			SV.arr[ctr + 800] = Vector3d(0,0,0) ;
			SV.arr[ctr+1200] =  Vector3d(0,0,0) ;	
			ctr++;
		}

	}	  
	
	/*
	Ball.Position = Vector3d(-8,15,-5);
	Ball.Velocity = Vector3d(0,0,0);
	Ball.Acceleration = gravity;
	*/
	for(int x= 0 ; x< 20 ; x++ )
	{			
		for(int z=0 ; z<20 ; z++ )
		{
			collisionMatrix1[x][z] = 0;
			collisionMatrix2[x][z] = 0;
		}

	}
}


// draws a simple grid
void makeGrid() {


//GLfloat lightpos1[] = {0, 20, 0, 1};
//	glLightfv(GL_LIGHT0, GL_POSITION, lightpos1);
		
	
	
	// Set up texture for the table

	glBindTexture(GL_TEXTURE_2D,Texture[0]);

	

	// Draw the table

	glBegin(GL_QUADS);
	glNormal3d(0, 1, 0);

	/*
	glTexCoord2f(0.0f,0.0f); glVertex3f(-12,-11,6);

	glTexCoord2f(1.0f,1.0f); glVertex3f(-12,-11,-12);

	glTexCoord2f(1.0f,0.0f); glVertex3f(12,-11,-12);

	glTexCoord2f(0.0f,1.0f); glVertex3f(12,-11,6);*/
	
	/*
	glTexCoord2f(0.0f,0.0f); glVertex3f(-50,-12,35);

	glTexCoord2f(1.0f,0.0f); glVertex3f(-50,-12,-65);

	glTexCoord2f(0.0f,1.0f); glVertex3f(50,-12,-65);

	glTexCoord2f(1.0f,1.0f); glVertex3f(50,-12,35);*/
	
	glTexCoord2f(0.0f,0.0f); glVertex3f(-50,YFloor,35);

	glTexCoord2f(1.0f,0.0f); glVertex3f(-50,YFloor,-65);

	glTexCoord2f(0.0f,1.0f); glVertex3f(50,YFloor,-65);

	glTexCoord2f(1.0f,1.0f); glVertex3f(50,YFloor,35);

	glEnd();
	

}


//void drawSphere(double x, double y, double z)
void drawSphere()
{
	/*
	glColor3f(   0.0,  0.0, 1.0 );
	glTranslatef(x,y,z);
	glutSolidSphere(RADIUS,100,100);
	glTranslatef(-x,-y,-z);
	*/
	
	glPushMatrix();

	glTranslatef(RB.SV.position.x , RB.SV.position.y , RB.SV.position.z);
	//glTranslatef(x,y,z);
	
	RB.SV.q = RB.SV.q.normalize();
	
	//cout<<RB.SV.q<<endl;
	
	RB.SV.q.GLrotate();

	GLUquadricObj *qObj = gluNewQuadric();

	gluQuadricNormals(qObj, GLU_SMOOTH);

	gluQuadricTexture(qObj, GL_TRUE);

	glBindTexture(GL_TEXTURE_2D, Texture[1]);

	gluSphere(qObj, RADIUS, 100, 100);

	gluDeleteQuadric(qObj);

	glPopMatrix();
	
	
}

void calculateAcceleration(StateVector SVpassed)
{

	//Due to spring force

	Vector3d distanceVector;
	float distance=0.0;
	
	
	//copy position and velocity from the state vector passed
	
	int ctr=0;

	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			
			trampoline1[i][j].Position = SVpassed.arr[ctr];
			trampoline1[i][j].Velocity = SVpassed.arr[ctr+800];
					
			++ctr;
		}
	}
	
	//tramp1
	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			if(i<9)
			{
				Kspring =  Kspring +  ((KspringMax - Kspring )* (10 - i)/10 ) ;
			}
			else if(i>10)
			{
				Kspring =  Kspring +  ((KspringMax - Kspring )* (i-10)/10 ) ;
			}
			else
			{
				Kspring = Kspring * 1;
			}
		
			trampoline1[i][j].Acceleration =  Vector3d(0,0,0);
		
			if(i==0 || i==19  || j==0 || j==19)		//first and last rows and  columns are fixed
			{
				trampoline1[i][j].Acceleration = Vector3d(0,0,0);
			}	
			else
			{
				trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + gravity ;		//due to gravity
				
						
				
				//Due to adjacent particles
				
				
				
				//due to left particle
				
				distanceVector = trampoline1[i][j-1].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
											
				//if( distance > horizontalRestDistance)
				if( distance != horizontalRestDistance)
				{	
					//spring force
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i][j-1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				
				//due to right particle
				
				distanceVector = trampoline1[i][j+1].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
				
								
				//if( distance > horizontalRestDistance)
				if( distance != horizontalRestDistance)
				{
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
				
					//damping
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i][j+1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				//due to upper particle
				
				distanceVector = trampoline1[i-1][j].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
										
				//if( distance > verticalRestDistance)
				if( distance != verticalRestDistance)
				{
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
				
					//damping
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i-1][j].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				//due to lower particle
				
				distanceVector = trampoline1[i+1][j].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
					
					//if( distance > verticalRestDistance)
					if( distance != verticalRestDistance)
					{
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
					
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i+1][j].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
				
				// due to upper right particle
				
				distanceVector = trampoline1[i-1][j+1].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i-1][j+1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				// due to upper left particle
				
				distanceVector = trampoline1[i-1][j-1].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i-1][j-1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				// due to bottom right particle
				
				distanceVector = trampoline1[i+1][j+1].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i+1][j+1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				// due to bottom left particle
				
				distanceVector = trampoline1[i+1][j-1].Position - trampoline1[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i+1][j-1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				
				
				//due to collision (according to point collision strategy)
				if(collisionMatrix1[i][j]==1)  // if collision happened in the previous timestep
				{
					//distanceVector = Ball.Position - trampoline1[i][j].Position;
					distanceVector = RB.SV.position - trampoline1[i][j].Position;
					
					//Vector3d force = ((trampoline1[i][j].Acceleration - gravity ) * (trampoline1[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
					
					//calculate the total force on the particle in the normal direction i.e towards centre of the ball
					Vector3d force = ((trampoline1[i][j].Acceleration) * (trampoline1[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
									
				//Ball.Acceleration = Ball.Acceleration + (force/Ball.mass) ;
								
				trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration - force/trampoline1[i][j].mass ;
				
				}
				
			}
		}
	}
	  
	  
	  
	  
	distance=0.0;
	
	
	//copy position and velocity from the state vector passed
	
	ctr=0;

	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			
			trampoline2[i][j].Position = SVpassed.arr[ctr+400];
			trampoline2[i][j].Velocity = SVpassed.arr[ctr+1200];
					
			++ctr;
		}
	}
	
	//tramp2
	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			if(i<9)
			{
				Kspring =  Kspring +  ((KspringMax - Kspring )* (10 - i)/10 ) ;
			}
			else if(i>10)
			{
				Kspring =  Kspring +  ((KspringMax - Kspring )* (i-10)/10 ) ;
			}
			else
			{
				Kspring = Kspring * 1;
			}
		
		
			trampoline2[i][j].Acceleration =  Vector3d(0,0,0);
		
			if(i==0 || i==19  || j==0 || j==19)		//first and last rows and  columns are fixed
			{
				trampoline2[i][j].Acceleration = Vector3d(0,0,0);
			}	
			else
			{
				trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + gravity ;		//due to gravity
				
						
				
				//Due to adjacent particles
				
				
				
				//due to left particle
				
				distanceVector = trampoline2[i][j-1].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
											
				//if( distance > horizontalRestDistance)
				if( distance != horizontalRestDistance)
				{	
					//spring force
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i][j-1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				
				//due to right particle
				
				distanceVector = trampoline2[i][j+1].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
				
								
				//if( distance > horizontalRestDistance)
				if( distance != horizontalRestDistance)
				{
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
				
					//damping
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i][j+1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				//due to upper particle
				
				distanceVector = trampoline2[i-1][j].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
										
				//if( distance > verticalRestDistance)
				if( distance != verticalRestDistance)
				{
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
				
					//damping
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i-1][j].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				//due to lower particle
				
				distanceVector = trampoline2[i+1][j].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
					
					//if( distance > verticalRestDistance)
					if( distance != verticalRestDistance)
					{
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
					
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i+1][j].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
				
				// due to upper right particle
				
				distanceVector = trampoline2[i-1][j+1].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i-1][j+1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				// due to upper left particle
				
				distanceVector = trampoline2[i-1][j-1].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i-1][j-1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				// due to bottom right particle
				
				distanceVector = trampoline2[i+1][j+1].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i+1][j+1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				// due to bottom left particle
				
				distanceVector = trampoline2[i+1][j-1].Position - trampoline2[i][j].Position ;
					
				distance = distanceVector.norm();
				
				//if( distance > diagonalRestDistance)
				if( distance != diagonalRestDistance)
				{
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
					
					//damping
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i+1][j-1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
				}
				
				
				
				//due to collision (according to point collision strategy)
				if(collisionMatrix2[i][j]==1)  // if collision happened in the previous timestep
				{
					distanceVector = RB.SV.position - trampoline2[i][j].Position;
					
					//Vector3d force = ((trampoline2[i][j].Acceleration - gravity ) * (trampoline2[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
					
					//calculate the total force on the particle in the normal direction i.e towards centre of the ball
					Vector3d force = ((trampoline2[i][j].Acceleration) * (trampoline2[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
									
				//Ball.Acceleration = Ball.Acceleration + (force/Ball.mass) ;
								
				trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration - force/trampoline2[i][j].mass ;
				
				}
				
			}
		}
	}  
}


void detectCollision()
{
	Vector3d planeNormal , distanceVector;
	
	float distance;
	
	float difference;

	
	
	//with tramp1			//plane strategy
	/*
	for(int i=1; i<19 ; i++)
	{
		for(int j=1; j<19 ; j++)
		{
			//collision with plane whose lower right corner is tramp1[i][j]
			
			//1.find normal
			
			planeNormal = (trampoline1[i-1][j].Position - trampoline1[i][j].Position ) % (trampoline1[i][j-1].Position - trampoline1[i][j].Position ) ;
			
			planeNormal = planeNormal.normalize();
			
			//2. find distance of the center from a point on the plane
			
			distance = (Ball.Position - trampoline1[i][j].Position ) * planeNormal ;
			
			
			//3. check if the distance is less than the radius
			
			// if it is less than the radius , find out how much less it is 
			// shift all the points on that plane by the amount the distance is less along the normal
			if(distance < 0)
			{
				distance = distance * -1;
			}	
			
			if( distance < (RADIUS + EPSILON)  )
			{
				//std::cout<<"Collision";
			
				collisionMatrix1[i][j] = 1;
			
				difference = (RADIUS + EPSILON) - distance ;
				
				trampoline1[i][j].Position = trampoline1[i][j].Position + (difference) * ( -1 *planeNormal ) ;
				
				if(i!=1 && 1!=18 && j!=1 && j!=19)
				{
					trampoline1[i-1][j].Position = trampoline1[i-1][j].Position + (difference) * ( -1 *planeNormal ) ;
					
					trampoline1[i-1][j-1].Position = trampoline1[i-1][j-1].Position + (difference) * ( -1 *planeNormal ) ;
					
					trampoline1[i][j-1].Position = trampoline1[i][j-1].Position + (difference) * ( -1 *planeNormal ) ;
				}
			}
			else
			{
				collisionMatrix1[i][j] = 0;
			}
			
		}
	}*/
	
	//point collision strategy tramp1
	
	for(int i=1; i<19 ; i++)
	{
		for(int j=1; j<19 ; j++)
		{
			//step 1 - find the distance of the particular point from ball	
			distanceVector = RB.SV.position - trampoline1[i][j].Position ;
			distance = distanceVector.norm();
			
			//step 2 - check if that distance is less than the radius
			if(distance < (RADIUS + EPSILON))
			{
				collisionMatrix1[i][j] = 1;
				
				//step 3 - place the point back on the surface of the sphere 
				
				  trampoline1[i][j].Position = RB.SV.position +  (RADIUS+ 6.5*EPSILON) * (-1 * distanceVector.normalize());
			}
			else
			{
				collisionMatrix1[i][j] = 0;
			}
		}
	}
	
	//point collision strategy tramp2
	for(int i=1; i<19 ; i++)
	{
		for(int j=1; j<19 ; j++)
		{
			//step 1 - find the distance of the particular point from ball	
			distanceVector = RB.SV.position - trampoline2[i][j].Position ;
			distance = distanceVector.norm();
			
			//step 2 - check if that distance is less than the radius
			if(distance < (RADIUS + EPSILON))
			{
				collisionMatrix2[i][j] = 1;
				
				//step 3 - place the point back on the surface of the sphere 
				
				  trampoline2[i][j].Position = RB.SV.position +  (RADIUS+ 6.75*EPSILON) * (-1 * distanceVector.normalize());
			}
			else
			{
				collisionMatrix2[i][j] = 0;
			}
		}
	}
}

StateVector func(StateVector XX, float t)
{
	
	StateVector Xdot;
	
	calculateAcceleration(XX);
		
	for(int i=0; i< 800 ; i++)
	{
		Xdot.arr[i] = XX.arr[800 + i]; //copy velocity
	}
			
	
	int ctr =0;
	
	for(int i =0 ; i<20 ; i++)	
	{
		for(int j =0 ; j<20 ; j++)	
		{
			Xdot.arr[800 + ctr] = trampoline1[i][j].Acceleration ;
			ctr++;
		}	
	}
	
	
	ctr =0 ;
	for(int i =0 ; i<20 ; i++)	
	{
		for(int j =0 ; j<20 ; j++)	
		{
			Xdot.arr[1200 + ctr] = trampoline2[i][j].Acceleration ;
			ctr++;
		}	
	}
	
	return Xdot;
}

//Vector3d calculateAccelerationBall()
Vector3d calculateAccelerationBall(StateVectorBall XX)
{	/*
	Ball.Acceleration = gravity;
	     
	Vector3d distanceVector;
	  
	for(int i=0 ; i<yLength1*ydensity1 ; i++)
	{
		for(int j=0; j<zLength1*zdensity1 ; j++)
		{
			if(collisionMatrix[i][j]==1)
			{
				distanceVector = Ball.Position - trampoline1[i][j].Position;
				
				calculateAcceleration(SV);
				Vector3d force = ((trampoline1[i][j].Acceleration - gravity ) * (trampoline1[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
								
			Ball.Acceleration = Ball.Acceleration + (force/Ball.mass) ;
							
			//trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration - force/trampoline1[i][j].mass ;
			
			}
		}
	}*/
	
///*******************************************	
	//Ball.Acceleration = gravity;
	Vector3d Acceleration;
	
	Acceleration = gravity;
	
	
	//Due to spring force

	Vector3d distanceVector;
	float distance=0.0;
	
	
	//copy position and velocity from the state vector passed
	
	int ctr=0;

	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			
			trampoline1[i][j].Position = SV.arr[ctr];
			trampoline1[i][j].Velocity = SV.arr[ctr+800];
					
			++ctr;
		}
	}
	
	//tramp1
	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			if(collisionMatrix1[i][j]==1  && i!=0 && i!=19 && j!=0 && j!=19)  // if collision happened in the previous timestep
			{
				trampoline1[i][j].Acceleration =  Vector3d(0,0,0);
				/*
				if(i==0 || i==19  || j==0 || j==19)		//first and last rows and  columns are fixed
				{
					trampoline1[i][j].Acceleration = Vector3d(0,0,0);
				}	
				else*/
				{
					trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + gravity ;		//due to gravity
					
							
					
					//Due to adjacent particles
					
					
					
					//due to left particle
					
					distanceVector = trampoline1[i][j-1].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
												
					//if( distance > horizontalRestDistance)
					if( distance != horizontalRestDistance)
					{	
						//spring force
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i][j-1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					
					//due to right particle
					
					distanceVector = trampoline1[i][j+1].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
					
									
					//if( distance > horizontalRestDistance)
					if( distance != horizontalRestDistance)
					{
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
					
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i][j+1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					//due to upper particle
					
					distanceVector = trampoline1[i-1][j].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
											
					//if( distance > verticalRestDistance)
					if( distance != verticalRestDistance)
					{
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
					
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i-1][j].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					//due to lower particle
					
					distanceVector = trampoline1[i+1][j].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
						
						//if( distance > verticalRestDistance)
						if( distance != verticalRestDistance)
						{
							trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
						
							//damping
							trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i+1][j].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
						}
					
					// due to upper right particle
					
					distanceVector = trampoline1[i-1][j+1].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i-1][j+1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					// due to upper left particle
					
					distanceVector = trampoline1[i-1][j-1].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i-1][j-1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					// due to bottom right particle
					
					distanceVector = trampoline1[i+1][j+1].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i+1][j+1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					// due to bottom left particle
					
					distanceVector = trampoline1[i+1][j-1].Position - trampoline1[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kspring/trampoline1[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline1[i][j].Acceleration = trampoline1[i][j].Acceleration + (Kdamping/trampoline1[i][j].mass) * ( (trampoline1[i+1][j-1].Velocity - trampoline1[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
				}
				
				//till here acceleration on the collided particle has been calculated
				
				// now we will apply the normal component of that acceleration to the ball
				/*
				distanceVector = Ball.Position - trampoline1[i][j].Position;
										
				//calculate the total force on the particle in the normal direction i.e towards centre of the ball
				Vector3d force = ((trampoline1[i][j].Acceleration) * (trampoline1[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
				//Vector3d force = ((trampoline1[i][j].Acceleration-gravity) * (trampoline1[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
									
				Ball.Acceleration = Ball.Acceleration + (force/Ball.mass) ;
				*/
				
				//distanceVector = RB.SV.position - trampoline1[i][j].Position;						
				distanceVector = XX.position - trampoline1[i][j].Position;
				
				//calculate the total force on the particle in the normal direction i.e towards centre of the ball
				Vector3d force = ((trampoline1[i][j].Acceleration) * (trampoline1[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
				//Vector3d force = ((trampoline1[i][j].Acceleration-gravity) * (trampoline1[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
									
				Acceleration = Acceleration + (force/RB.mass) ;
			}	
		}
	}
	
	
	
	
	ctr=0;

	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			
			trampoline2[i][j].Position = SV.arr[ctr+400];
			trampoline2[i][j].Velocity = SV.arr[ctr+1200];
					
			++ctr;
		}
	}
	
	//tramp2
	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			if(collisionMatrix2[i][j]==1  && i!=0 && i!=19 && j!=0 && j!=19)  // if collision happened in the previous timestep
			{
				trampoline2[i][j].Acceleration =  Vector3d(0,0,0);
				/*
				if(i==0 || i==19  || j==0 || j==19)		//first and last rows and  columns are fixed
				{
					trampoline2[i][j].Acceleration = Vector3d(0,0,0);
				}	
				else*/
				{
					trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + gravity ;		//due to gravity
					
							
					
					//Due to adjacent particles
					
					
					
					//due to left particle
					
					distanceVector = trampoline2[i][j-1].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
												
					//if( distance > horizontalRestDistance)
					if( distance != horizontalRestDistance)
					{	
						//spring force
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i][j-1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					
					//due to right particle
					
					distanceVector = trampoline2[i][j+1].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
					
									
					//if( distance > horizontalRestDistance)
					if( distance != horizontalRestDistance)
					{
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - horizontalRestDistance)  * distanceVector.normalize() ;
					
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i][j+1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					//due to upper particle
					
					distanceVector = trampoline2[i-1][j].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
											
					//if( distance > verticalRestDistance)
					if( distance != verticalRestDistance)
					{
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
					
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i-1][j].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					//due to lower particle
					
					distanceVector = trampoline2[i+1][j].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
						
						//if( distance > verticalRestDistance)
						if( distance != verticalRestDistance)
						{
							trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - verticalRestDistance)  * distanceVector.normalize() ;
						
							//damping
							trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i+1][j].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
						}
					
					// due to upper right particle
					
					distanceVector = trampoline2[i-1][j+1].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i-1][j+1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					// due to upper left particle
					
					distanceVector = trampoline2[i-1][j-1].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i-1][j-1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					// due to bottom right particle
					
					distanceVector = trampoline2[i+1][j+1].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i+1][j+1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
					
					// due to bottom left particle
					
					distanceVector = trampoline2[i+1][j-1].Position - trampoline2[i][j].Position ;
						
					distance = distanceVector.norm();
					
					//if( distance > diagonalRestDistance)
					if( distance != diagonalRestDistance)
					{
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kspring/trampoline2[i][j].mass) * (distance - diagonalRestDistance)  * distanceVector.normalize() ;
						
						//damping
						trampoline2[i][j].Acceleration = trampoline2[i][j].Acceleration + (Kdamping/trampoline2[i][j].mass) * ( (trampoline2[i+1][j-1].Velocity - trampoline2[i][j].Velocity)*(distanceVector.normalize()) )  * distanceVector.normalize() ;
					}
					
				}
				
				//till here acceleration on the collided particle has been calculated
				
				// now we will apply the normal component of that acceleration to the ball
				/*
				distanceVector = Ball.Position - trampoline2[i][j].Position;
										
				//calculate the total force on the particle in the normal direction i.e towards centre of the ball
				Vector3d force = ((trampoline2[i][j].Acceleration) * (trampoline2[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
				//Vector3d force = ((trampoline2[i][j].Acceleration-gravity) * (trampoline2[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
									
				Ball.Acceleration = Ball.Acceleration + (force/Ball.mass) ;
				*/
				
				//distanceVector = RB.SV.position - trampoline2[i][j].Position;
				distanceVector = XX.position - trampoline2[i][j].Position;
										
				//calculate the total force on the particle in the normal direction i.e towards centre of the ball
				Vector3d force = ((trampoline2[i][j].Acceleration) * (trampoline2[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
				//Vector3d force = ((trampoline2[i][j].Acceleration-gravity) * (trampoline2[i][j].mass) * (distanceVector.normalize())) *  (distanceVector.normalize());
									
				Acceleration = Acceleration + (force/RB.mass) ;
			}	
		}
	}
	
	
	return Acceleration;
}


Vector3d calculateTorqueDueToCollision(StateVectorBall XX)
{
	Vector3d normal , torque(0,0,0) , r;
	

	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			if(collisionMatrix1[i][j]==1  && i!=0 && i!=19 && j!=0 && j!=19)  // if collision happened in the previous timestep
			{
				normal = XX.position - trampoline1[i][j].Position ; 
				normal = normal.normalize();
				
				r = RADIUS * normal * -1 ;
				
				Vector3d VelocityBallPoint = (RB.SV.momentum / RB.mass) + RB.omega%r;
				
				//Vector3d Vrel = VelocityBallPoint - SV[800 + (i*20 + j )  ] ; //velocity of point on ball - velocity of [i][j] particle
				
				Vector3d Vrel = VelocityBallPoint - trampoline1[i][j].Velocity ; //velocity of point on ball - velocity of [i][j] particle
				
				torque = torque + (-1 * normal * RADIUS) % ( -30 * Friction * Vrel ) ;  
				
			}
		}
	}
	
	for(int i=0 ; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			if(collisionMatrix2[i][j]==1  && i!=0 && i!=19 && j!=0 && j!=19)  // if collision happened in the previous timestep
			{
				normal = XX.position - trampoline2[i][j].Position ; 
				normal = normal.normalize();
				
				r = RADIUS * normal * -1 ;
				
				Vector3d VelocityBallPoint = (RB.SV.momentum / RB.mass) + RB.omega%r;
				
				//Vector3d Vrel = VelocityBallPoint - SV[800 + (i*20 + j )  ] ; //velocity of point on ball - velocity of [i][j] particle
				
				Vector3d Vrel = VelocityBallPoint - trampoline1[i][j].Velocity ; //velocity of point on ball - velocity of [i][j] particle
				
				torque = torque + (-1 * normal * RADIUS) % ( -100 * Friction * Vrel ) ;  
				
			}
		}
	}
	
	return torque;
}

StateVectorBall funcBall(StateVectorBall XX, float t)
{
	StateVectorBall Xdot;
	
		
	Vector3d Velocity, force, torque(0,0,0), omega ;
	Quaternion qdot, omegaQuaternion;
	
	Matrix3x3 rotation,  Iinv;
	
	
	//Step 1
	Xdot.position = XX.momentum / (RB.mass) ;
	
	//Step 2
	XX.q = XX.q.normalize();
	rotation = XX.q.rotation();
	
	//Step 3
	Iinv = rotation * RB.Ibodyinv * rotation.transpose();
	
	//Step 4
	omega = Iinv * XX.angularMomentum ;
	
	//Step 5
	omegaQuaternion.set( omega ) ;
	
	//Step 6
	qdot = (0.5) * omegaQuaternion * XX.q ;
	Xdot.q = qdot;	
		
	//Step 7
	//force = gravity * RB.mass;
	//force = calculateAccelerationBall() * RB.mass;
	force = calculateAccelerationBall(XX) * RB.mass;
	Xdot.momentum = force;
	
	//Step 8
	//Xdot.angularMomentum = torque;
	Xdot.angularMomentum = calculateTorqueDueToCollision(XX)	;
	return Xdot;
}


void updateState(float dt)
{
	
	
	StateVector X;
	StateVector K1, K2, K3, K4;
	
	StateVectorBall Y ;
	StateVectorBall Y1, Y2, Y3, Y4;
	
	
	float t=0;
	
	
	//Y = SVBall;
	Y = RB.SV;
		
	Y1 = funcBall(Y,dt) * dt;
	
	Y2 = funcBall(  Y + (Y1*0.5) , t + (dt*0.5) )* dt;
	
	Y3 = funcBall( Y + (Y2*0.5) , t + (dt*0.5) ) * dt;
	
	Y4 = funcBall( Y + Y3 , t + dt)* dt;
		
	Y = Y + ( Y1 + Y2*2 + Y3*2 + Y4)*(0.1667);
	
	//Y = Y +  Y1 ;
	
	//SVBall = Y;	
	RB.SV = Y;	
	
	
	RB.SV.q = RB.SV.q.normalize();
	
	RB.R = RB.SV.q.rotation();
	
	RB.Iinv = RB.R * RB.Ibodyinv * RB.R.transpose() ;
	
	RB.omega = RB.Iinv * RB.SV.angularMomentum ;
	
	
	
	/*
	Ball.Position = Ball.Position + (Ball.Velocity * TimerDelay * 0.001);
	
	// this function calculates the aceeleration on the ball based on the position of springs in the previous timestep. if collisions occured in the previous timestep , it adds the normal component of the forces on the springs to the acceleration of the ball
	calculateAccelerationBall();  
	
	Ball.Velocity = Ball.Velocity + (Ball.Acceleration *TimerDelay * 0.001);
	*/
	
	
	
	X = SV;
	
		
	K1 = func(X,dt) * dt;
	
	K2 = func(  X + (K1*0.5) , t + (dt*0.5) )* dt;
	
	K3 = func( X + (K2*0.5) , t + (dt*0.5) ) * dt;
	
	K4 = func( X + K3 , t + dt)* dt;
		
	X = X + ( K1 + K2*2 + K3*2 + K4)*(0.1667);
	
	//X = X +  K1 ;
	
	SV = X;	
	
	
	for(int i=800 ; i<1600 ; i++)
	{
		SV.arr[i] = 0.98 * SV.arr[i];
	}
	
}


void drawTrampolines()
{


	
	//lines
	
	glColor3f(   1.0,  1.0, 1.0 );
	
	glLineWidth(2.5f);
	
	for(int i=0 ; i<19 ;i++)
	{
		for(int j=0; j<19 ;j++)
		{
			//horizontal lines
			if(i!=0)	
			{
				glBegin(GL_LINES);
				//glColor3f(   1.0,  0.0, 0.0 );
				glVertex3f(trampoline1[i][j].Position.x, trampoline1[i][j].Position.y , trampoline1[i][j].Position.z);
				glVertex3f(trampoline1[i][j+1].Position.x, trampoline1[i][j+1].Position.y , trampoline1[i][j+1].Position.z);
				glEnd();
			}
			
			//vertical lines
			if(j!=0)
			{
				glBegin(GL_LINES);
				//glColor3f(   1.0,  0.0, 0.0 );
				glVertex3f(trampoline1[i][j].Position.x, trampoline1[i][j].Position.y , trampoline1[i][j].Position.z);
				glVertex3f(trampoline1[i+1][j].Position.x, trampoline1[i+1][j].Position.y , trampoline1[i+1][j].Position.z);
				glEnd();
			}
			
		}
		
	}
	
	glColor3f(   0.85,  0.0, 0.0 );
	
	glLineWidth(10.5f);
	
	glBegin(GL_LINES);
	glVertex3f(trampoline1[0][0].Position.x, trampoline1[0][0].Position.y , trampoline1[0][0].Position.z);
	glVertex3f(trampoline1[0][19].Position.x, trampoline1[0][19].Position.y , trampoline1[0][19].Position.z);
	glEnd();
	
	glBegin(GL_LINES);
	glVertex3f(trampoline1[0][19].Position.x, trampoline1[0][19].Position.y , trampoline1[0][19].Position.z);
	glVertex3f(trampoline1[19][19].Position.x, trampoline1[19][19].Position.y , trampoline1[19][19].Position.z);
	glEnd();
	
	glBegin(GL_LINES);
	glVertex3f(trampoline1[19][19].Position.x, trampoline1[19][19].Position.y , trampoline1[19][19].Position.z);
	glVertex3f(trampoline1[19][0].Position.x, trampoline1[19][0].Position.y , trampoline1[19][0].Position.z);
	glEnd();
	 
	glBegin(GL_LINES);
	glVertex3f(trampoline1[19][0].Position.x, trampoline1[19][0].Position.y , trampoline1[19][0].Position.z);
	glVertex3f(trampoline1[0][0].Position.x, trampoline1[0][0].Position.y , trampoline1[0][0].Position.z);
	glEnd(); 
	 
	
	 

	  glColor3f(   1.0,  1.0, 1.0 );
	
	glLineWidth(2.5f);
	  
	  
	 for(int i=0 ; i<19 ;i++)
	{
		for(int j=0; j<19 ;j++)
		{
			//horizontal lines
			if(i!=0)	
			{
				glBegin(GL_LINES);
				//glColor3f(   0.0,  1.0, 0.0 );
				glVertex3f(trampoline2[i][j].Position.x, trampoline2[i][j].Position.y , trampoline2[i][j].Position.z);
				glVertex3f(trampoline2[i][j+1].Position.x, trampoline2[i][j+1].Position.y , trampoline2[i][j+1].Position.z);
				glEnd();
			}
			
			//vertical lines
			if(j!=0)
			{
				glBegin(GL_LINES);
				//glColor3f(   0.0,  1.0, 0.0 );
				glVertex3f(trampoline2[i][j].Position.x, trampoline2[i][j].Position.y , trampoline2[i][j].Position.z);
				glVertex3f(trampoline2[i+1][j].Position.x, trampoline2[i+1][j].Position.y , trampoline2[i+1][j].Position.z);
				glEnd();
			}
			
		}
		
	}
	
	
	
	glColor3f(   0.85,  0.0, 0.0 );
	
	glLineWidth(10.5f);
	
	glBegin(GL_LINES);
	glVertex3f(trampoline2[0][0].Position.x, trampoline2[0][0].Position.y , trampoline2[0][0].Position.z);
	glVertex3f(trampoline2[0][19].Position.x, trampoline2[0][19].Position.y , trampoline2[0][19].Position.z);
	glEnd();
	
	glBegin(GL_LINES);
	glVertex3f(trampoline2[0][19].Position.x, trampoline2[0][19].Position.y , trampoline2[0][19].Position.z);
	glVertex3f(trampoline2[19][19].Position.x, trampoline2[19][19].Position.y , trampoline2[19][19].Position.z);
	glEnd();
	
	glBegin(GL_LINES);
	glVertex3f(trampoline2[19][19].Position.x, trampoline2[19][19].Position.y , trampoline2[19][19].Position.z);
	glVertex3f(trampoline2[19][0].Position.x, trampoline2[19][0].Position.y , trampoline2[19][0].Position.z);
	glEnd();
	 
	glBegin(GL_LINES);
	glVertex3f(trampoline2[19][0].Position.x, trampoline2[19][0].Position.y , trampoline2[19][0].Position.z);
	glVertex3f(trampoline2[0][0].Position.x, trampoline2[0][0].Position.y , trampoline2[0][0].Position.z);
	glEnd(); 
	
	
	//cloth
	/*
	for(int i=0 ; i<19 ;i++)
	{
		for(int j=0; j<19 ;j++)
		{
			
			glBegin(GL_QUADS);
			
			if(j%2==0)
			glColor3f(   1.0,  0.0, 0.0 );
			else
			glColor3f(   1.0,  1.0, 1.0 );
			
			glVertex3f(trampoline1[i][j].Position.x, trampoline1[i][j].Position.y , trampoline1[i][j].Position.z);
			
			glVertex3f(trampoline1[i][j+1].Position.x, trampoline1[i][j+1].Position.y , trampoline1[i][j+1].Position.z);
			
			glVertex3f(trampoline1[i+1][j+1].Position.x, trampoline1[i+1][j+1].Position.y , trampoline1[i+1][j+1].Position.z);
			
			glVertex3f(trampoline1[i+1][j].Position.x, trampoline1[i+1][j].Position.y , trampoline1[i+1][j].Position.z);
			
			glEnd();
			
		}
		
		for(int i=0 ; i<19 ;i++)
		{
			for(int j=0; j<19 ;j++)
			{
				
				glBegin(GL_QUADS);
				
				if(j%2==0)
				glColor3f(   0.0,  1.0, 0.0 );
				else
				glColor3f(   1.0,  1.0, 1.0 );
				
				glVertex3f(trampoline2[i][j].Position.x, trampoline2[i][j].Position.y , trampoline2[i][j].Position.z);
				
				glVertex3f(trampoline2[i][j+1].Position.x, trampoline2[i][j+1].Position.y , trampoline2[i][j+1].Position.z);
				
				glVertex3f(trampoline2[i+1][j+1].Position.x, trampoline2[i+1][j+1].Position.y , trampoline2[i+1][j+1].Position.z);
				
				glVertex3f(trampoline2[i+1][j].Position.x, trampoline2[i+1][j].Position.y , trampoline2[i+1][j].Position.z);
				
				glEnd();
				
			}
			
		}*/
		/*
		glBegin(GL_POINTS);
		
		glColor3f(   0.0,  1.0, 0.0 );
		
		for(int i=0 ; i<19 ;i++)
		{
			for(int j=0; j<19 ;j++)
			{
				
				glVertex3f(trampoline1[i][j].Position.x, trampoline1[i][j].Position.y , trampoline1[i][j].Position.z);
									
			}
			
		}
	  
	  
		for(int i=0 ; i<19 ;i++)
		{
			for(int j=0; j<19 ;j++)
			{	
				glVertex3f(trampoline2[i][j].Position.x, trampoline2[i][j].Position.y , trampoline2[i][j].Position.z);	
			}
			
		}
		glEnd();
	*/
	
	//drawSphere(RB.SV.position.x,RB.SV.position.y,RB.SV.position.z);
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	glEnable(GL_TEXTURE_2D);
	
	if (showGrid) 
    makeGrid();
	
	drawSphere();
	
	glDisable(GL_TEXTURE_2D);

/*
	  string filename = "Screens/ScreenShot.";
	  filename += int_to_string(framecount);
	  filename += ".tga";
	  framecount++;
	  ScreenShot(filename, 800, 600, false);*/
	//ScreenShot(filename, 1920, 1080, false);
}


void copySVtoParticles()
{
	int ctr = 0;

	for(int i=0; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			trampoline1[i][j].Position = SV.arr[ctr];
			trampoline1[i][j].Velocity = SV.arr[ctr+800];
			trampoline2[i][j].Position = SV.arr[ctr+400];
			trampoline2[i][j].Velocity = SV.arr[ctr+1200];
			ctr++;
		}
	}	

}

void copyParticlestoSV()
{
	int ctr = 0;

	for(int i=0; i<20 ; i++)
	{
		for(int j=0; j<20 ; j++)
		{
			
			SV.arr[ctr] = trampoline1[i][j].Position ;
			SV.arr[ctr+800] = trampoline1[i][j].Velocity ;
			SV.arr[ctr+400] = trampoline2[i][j].Position ;
			SV.arr[ctr+1200] = trampoline2[i][j].Velocity ;
				
			ctr++;
		}
	}
}


void detectCollisionWithFloor()
{
	float Numerator, Denominator , j;
	Vector3d surfaceNormal(0,1,0) , r , deltaV(0,0,0) , deltaW(0,0,0), omega , newV , newW, J , T;
	float VrelMag;
	Vector3d Corners[8];
	
	float difference;
	
	//for(int i=0 ; i< SIZE ; i++)
	//{
		deltaV.set(0,0,0) ;
		deltaW.set(0,0,0);
		
		//for(int k=0 ;k <8 ; k++)
		//{	
			//Corners[k] = RB.SV.position + RB.R*Positions[k] ;
			
			//if(fabs(Corners[k].y) < (EPSILON))
			//if(Corners[k].y < (EPSILON))			
			
			if( (RB.SV.position.y - RADIUS) < ( YFloor + FloorEpsilon))			
			{
				difference =  ( YFloor + FloorEpsilon ) - (RB.SV.position.y - RADIUS) ;
			
				RB.SV.position.y = RB.SV.position.y + difference 	;
			
				//Corners[k] = RB.SV.position + RB.R*Positions[k] ;
			
				//r = Corners[k] - RB.SV.position ;
				r = (RADIUS * surfaceNormal * -1 );
							
				//relative velocity in the direction of normal //add w*r
							
				Vector3d Vrel = (RB.SV.momentum / RB.mass) + RB.omega%r;			
							
				VrelMag = Vrel*surfaceNormal;	
				
				
				
				if(VrelMag<0)
				{
					Numerator = - (1 + Restitution) * VrelMag ;	
					
											
					float part = surfaceNormal*( ( RB.Iinv * (r%surfaceNormal) ) % r) ;
					
											
					Denominator = (1/RB.mass) +  part ;
					
					j = Numerator/Denominator ;
					
					J = j * surfaceNormal ;
					
					RB.SV.momentum = RB.SV.momentum + J ;
					
					/*
					T = r%J;
					
					RB.SV.angularMomentum = RB.SV.angularMomentum  + T ; 
					
					
					if(j<0.10)
					{
						j=0;
						
						RB.SV.angularMomentum = 0.5 * RB.SV.angularMomentum ;
						RB.SV.momentum = 0.5 * RB.SV.momentum ;
						
						if(RB.SV.angularMomentum.norm() < LeastValue && RB.SV.momentum.norm()< LeastValue)
						{
							RB.SV.angularMomentum = 0;
						RB.SV.momentum = 0;
						}
					}*/
					
					
					/*
					normal = RB.SV.position - trampoline1[i][j].Position ; 
					normal = normal.normalize();
					
					r = RADIUS * normal * -1 ;
					
					Vector3d VelocityBallPoint = (RB.SV.momentum / RB.mass) + RB.omega%r;
					
					//Vector3d Vrel = VelocityBallPoint - SV[800 + (i*20 + j )  ] ; //velocity of point on ball - velocity of [i][j] particle
					
					Vector3d Vrel = VelocityBallPoint - trampoline1[i][j].Velocity ; //velocity of point on ball - velocity of [i][j] particle
					
					torque = torque + (-1 * normal * RADIUS) % ( -30 * Friction * Vrel ) ;  
					*/
					/*
					T = (RADIUS * -1 * surfaceNormal ) % ( -100 * FrictionFloor * Vrel ) ; 

					RB.SV.angularMomentum = RB.SV.angularMomentum  + T ; */
					
					RB.SV.angularMomentum = RB.SV.angularMomentum * 0.6;
					
				}
					
			}
		
		//}		
	//}
}



void PerspDisplay() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw the camera created in perspective
	camera->PerspectiveDisplay(WIDTH, HEIGHT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//if (showGrid) 
    //makeGrid();
  
  //
  // here is where you would draw your scene!
  
    
	updateState(TimerDelay * 0.001);
	
	//detectCollision();
	
	copySVtoParticles();			// SV is updated after updateState(), this new state is copied to Particles
	
	detectCollision();				//collisions are detected and positions of points are changed if they are colliding
		
	copyParticlestoSV();			// if any positions are changed due to collision , update those in state vector
	
	detectCollisionWithFloor();
	
	drawTrampolines();
	
	glutTimerFunc(TimerDelay, TimerCallback, 0);
 

	glutSwapBuffers();

		  string filename = "Screens/ScreenShot.";
	  filename += int_to_string(framecount);
	  filename += ".tga";
	  framecount++;
	  ScreenShot(filename, 1920, 1080, false);
}


void TimerCallback(int){

  PerspDisplay();
}

void mouseEventHandler(int button, int state, int x, int y) {
  // let the camera handle some specific mouse events (similar to maya)
  camera->HandleMouseEvent(button, state, x, y);
  glutPostRedisplay();
}

void motionEventHandler(int x, int y) {
  // let the camera handle some mouse motions if the camera is to be moved
  camera->HandleMouseMotion(x, y);
  glutPostRedisplay();
}



void LoadParameters(char *filename){
  
  
  float ballMass, trampMass;
  
FILE *paramfile;  

  if((paramfile = fopen(filename, "r")) == NULL){
    fprintf(stderr, "error opening parameter file %s\n", filename);
    exit(1);
  }

  ParamFilename = filename;

  if( fscanf(paramfile, "%f %f %f %f %f %f %f %f", &Kspring , &KspringMax ,  &Kdamping ,  &Restitution , &Friction , &ballMass , &trampMass, &RADIUS  ) != 8  )
	{
		fprintf(stderr, "error reading parameter file %s\n", filename);
		fclose(paramfile);
		exit(1);
	}
	
	
	RB.mass = ballMass;
	
	RB.Ibody = Matrix3x3(1,0,0,0,1,0,0,0,1) ;
	
	RB.Ibody = ( 0.4 * RB.mass * RADIUS * RADIUS ) * RB.Ibody ;
	
	RB.Ibodyinv = RB.Ibody.inv();
	
	
	
	for (int i =0 ;i <20 ; i++)
	{
		for (int j =0 ;j <20 ; j++)
		{
			trampoline1[i][j].mass = trampMass ;
			trampoline2[i][j].mass = trampMass ;
		}	
	}
}


void keyboardEventHandler(unsigned char key, int x, int y) {
  switch (key) {
  case 'r': case 'R':
    // reset the camera to its initial position
    camera->Reset();
    break;
	/*
  case 'f': case 'F':
    camera->SetCenterOfFocus(Vector3d(0, 0, 0));
    break;
  case 'g': case 'G':
    showGrid = !showGrid;
   break;*/
      
	 case 'u': case 'U':	// update 
	  
	LoadParameters(ParamFilename);
	  break;
	 /* 
	 case 'b': case 'B': 
	  Ball.Position = Vector3d(12,15,-5);
	Ball.Velocity = Vector3d(0,0,0);
	Ball.Acceleration = gravity;
	break;*/ 
 case 's': case 'S': 
	  RB.SV.position = Vector3d(-8,15,-5);
	RB.SV.momentum = Vector3d(0,0,0);
	RB.SV.angularMomentum = Vector3d(2,2,2);
	//RB.SV.acceleration = gravity;
	break; 	
	  
  case 'q': case 'Q':	// q or esc - quit
  case 27:		// esc
    exit(0);
  }

  glutPostRedisplay();
}




int main(int argc, char *argv[]) {

	LoadParameters(argv[1]);

  // set up opengl window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(50, 50);
  persp_win = glutCreateWindow("Boing!!");

  // initialize the camera and such
  init();

   
  
  // set up opengl callback functions
  glutDisplayFunc(PerspDisplay);
  glutMouseFunc(mouseEventHandler);
  glutMotionFunc(motionEventHandler);
  glutKeyboardFunc(keyboardEventHandler);

  
  
  glutMainLoop();
  return(0);
}

