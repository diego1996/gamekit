/*
GameKit
Copyright (c) 2009 Erwin Coumans  http://gamekit.googlecode.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "IrrBlendNew.h"
#include "BulletIrrlichtUtils.h"
#include "autogenerated/blender.h"
#include "bMain.h"
#include "bBlenderFile.h"



IrrBlendNew::IrrBlendNew(irr::IrrlichtDevice* device,irr::scene::ISceneManager* irrlichtSceneManager, btDiscreteDynamicsWorld* bulletWorld, btLogicManager* logicManager)
:BulletBlendReaderNew(bulletWorld),
m_device(device),
m_sceneManager(irrlichtSceneManager),
m_logicManager(logicManager)
{

}


IrrBlendNew::~IrrBlendNew()
{

}


void	IrrBlendNew::convertLogicBricks()
{
}


void	IrrBlendNew::createParentChildHierarchy()
{

	int i;
	btAlignedObjectArray<btCollisionObject*>children;
	
	for (i=0;i<this->m_destinationWorld->getNumCollisionObjects();i++)
	{
		btCollisionObject* childColObj = m_destinationWorld->getCollisionObjectArray()[i];
		IrrlichtMeshContainer* childNode = (IrrlichtMeshContainer*)childColObj->getUserPointer();
		if (!childNode)
			continue;
		Blender::Object* obj = (Blender::Object*)childNode->m_userPointer;
		if (!obj)
			continue;

		if (obj->parent)
		{
			///@todo: deal with 'parentinv'
			btCollisionObject* colParent = (btCollisionObject*) obj->parent->vnode;

			IrrlichtMeshContainer* parentNode = colParent ? (IrrlichtMeshContainer*) colParent->getUserPointer(): 0;

			if(parentNode && parentNode->m_sceneNodes.size())
			{
				for (int i=0;i<childNode->m_sceneNodes.size();i++)
				{
					childNode->m_sceneNodes[i]->setParent(parentNode->m_sceneNodes[0]);
				}
			}
			children.push_back(childColObj);
		}
	}


	for (i=0;i<children.size();i++)
	{
		btCollisionObject* childColObj = children[i];
		m_destinationWorld->removeCollisionObject(childColObj);
		///disconnect the rigidbody update for child objects
		childColObj->setUserPointer(0);
	}
}


	
//after each object is converter, including collision object, create a graphics object (and bind them)
void* IrrBlendNew::createGraphicsObject(Blender::Object* tmpObject, class btCollisionObject* bulletObject)
{

	IrrlichtMeshContainer* meshContainer = 0;
	bParse::bMain* mainPtr = m_blendFile->getMain();

	
	btRigidBody* body = bulletObject ? btRigidBody::upcast(bulletObject) : 0;
	IrrMotionState* newMotionState = 0;

	if (body)
	{
		if (!bulletObject->isStaticOrKinematicObject())
		{
			newMotionState = new IrrMotionState();
			newMotionState->setWorldTransform(body->getWorldTransform());
			body->setMotionState(newMotionState);
		}
	}

	Blender::Mesh *me = (Blender::Mesh*)tmpObject->data;

	if (me && me->totvert  && me->totface)
	{
		if (me->totvert> 16300)
		{
			printf("me->totvert = %d\n",me->totvert);
		}

		
		int maxVerts = btMin(16300,btMax(me->totface*3*2,(me->totvert-6)));

		irr::video::S3DVertex* orgVertices= new irr::video::S3DVertex[me->totvert];
		irr::video::S3DVertex* newVertices= new irr::video::S3DVertex[maxVerts];
		

		if (!me->mvert)
			return 0;

		for (int v=0;v<me->totvert;v++)
		{
			float* vt3 = &me->mvert[v].co.x;
			orgVertices[v] = irr::video::S3DVertex(	IRR_X_M*vt3[IRR_X],	IRR_Y_M*vt3[IRR_Y],	IRR_Z_M*vt3[IRR_Z], 1,1,0,		irr::video::SColor(255,255,255,255), 0, 1);
		}

		
		int numVertices = 0;
		int numTriangles=0;
		int numIndices = 0;
		int currentIndex = 0;

		

		int maxNumIndices = me->totface*4*2;

		unsigned short int* indices= new unsigned short int[maxNumIndices];

		for (int t=0;t<me->totface;t++)
		{
			if (currentIndex>maxNumIndices)
				break;

			int v[4] = {me->mface[t].v1,me->mface[t].v2,me->mface[t].v3,me->mface[t].v4};
		
			int originalIndex = v[IRR_TRI_0_X];
			indices[numIndices] = currentIndex;
			newVertices[indices[numIndices]] = orgVertices[originalIndex];
			if (me->mtface)
			{
				newVertices[indices[numIndices]].TCoords.X = me->mtface[t].uv[IRR_TRI_0_X][0];
				newVertices[indices[numIndices]].TCoords.Y = 1.f - me->mtface[t].uv[IRR_TRI_0_X][1];
			} else
			{
				newVertices[indices[numIndices]].TCoords.X = 0;
				newVertices[indices[numIndices]].TCoords.Y = 0;
			}
			numIndices++;
			currentIndex++;
			numVertices++;

			originalIndex = v[IRR_TRI_0_Y];
			indices[numIndices] = currentIndex;
			newVertices[indices[numIndices]] = orgVertices[originalIndex];
			if (me->mtface)
			{
				newVertices[indices[numIndices]].TCoords.X = me->mtface[t].uv[IRR_TRI_0_Y][0];
				newVertices[indices[numIndices]].TCoords.Y = 1.f - me->mtface[t].uv[IRR_TRI_0_Y][1];
			} else
			{
				newVertices[indices[numIndices]].TCoords.X = 0;
				newVertices[indices[numIndices]].TCoords.Y = 0;
			}
			numIndices++;
			currentIndex++;
			numVertices++;

			originalIndex = v[IRR_TRI_0_Z];
			indices[numIndices] = currentIndex;
			newVertices[indices[numIndices]] = orgVertices[originalIndex];
			if (me->mtface)
			{
				newVertices[indices[numIndices]].TCoords.X = me->mtface[t].uv[IRR_TRI_0_Z][0];
				newVertices[indices[numIndices]].TCoords.Y = 1.f - me->mtface[t].uv[IRR_TRI_0_Z][1];
			} else
			{
				newVertices[indices[numIndices]].TCoords.X = 0;
				newVertices[indices[numIndices]].TCoords.Y = 0;
			}
			numIndices++;
			currentIndex++;
			numVertices++;
			numTriangles++;

			if (v[3])
			{
				originalIndex = v[IRR_TRI_1_X];
				indices[numIndices]= currentIndex;
				newVertices[currentIndex] = orgVertices[originalIndex];
				if (me->mtface)
				{
					newVertices[currentIndex].TCoords.X = me->mtface[t].uv[IRR_TRI_1_X][0];
					newVertices[currentIndex].TCoords.Y = 1.f -  me->mtface[t].uv[IRR_TRI_1_X][1];
				} else
				{
					newVertices[currentIndex].TCoords.X = 0;
					newVertices[currentIndex].TCoords.Y = 0;
				}
				numIndices++;
				numVertices++;
				currentIndex++;

				originalIndex =v[IRR_TRI_1_Y];
				indices[numIndices]= currentIndex;
				newVertices[currentIndex] = orgVertices[originalIndex];
				if (me->mtface)
				{
					newVertices[currentIndex].TCoords.X = me->mtface[t].uv[IRR_TRI_1_Y][0];
					newVertices[currentIndex].TCoords.Y = 1.f - me->mtface[t].uv[IRR_TRI_1_Y][1];
				} else
				{
					newVertices[currentIndex].TCoords.X = 0;
					newVertices[currentIndex].TCoords.Y = 0;
				}
				numIndices++;
				numVertices++;
				currentIndex++;

				originalIndex = v[IRR_TRI_1_Z];
				indices[numIndices]= currentIndex;
				newVertices[currentIndex] = orgVertices[originalIndex];
				if (me->mtface)
				{
					newVertices[currentIndex].TCoords.X = me->mtface[t].uv[IRR_TRI_1_Z][0];
					newVertices[currentIndex].TCoords.Y = 1.f - me->mtface[t].uv[IRR_TRI_1_Z][1];
				} else
				{
					newVertices[currentIndex].TCoords.X = 0;
					newVertices[currentIndex].TCoords.Y = 0;
				}
				
				numIndices++;
				numVertices++;
				currentIndex++;

				numTriangles++;
			}

			
			///subdivide the mesh into smaller parts, so that indices fit in short int
			if (numVertices>=maxVerts)
			{
				irr::scene::ISceneNode* node = createMeshNode(newVertices,numVertices,indices,numIndices,numTriangles,bulletObject,tmpObject);
				if (!meshContainer)
					meshContainer = new IrrlichtMeshContainer();
				
				meshContainer->m_userPointer = tmpObject;
				meshContainer->m_sceneNodes.push_back(node);

				if (newMotionState && node)
					newMotionState->addIrrlichtNode(node);
				
				numVertices = 0;
				numTriangles = 0;
				numIndices = 0;

				numVertices = 0;
				currentIndex = 0;

				indices= new unsigned short int[me->totface*4*2];
			}
		}

		if (numTriangles>0)
		{
			irr::scene::ISceneNode* node = createMeshNode(newVertices,numVertices,indices,numIndices,numTriangles,bulletObject,tmpObject);

			if (!meshContainer)
					meshContainer = new IrrlichtMeshContainer();
					
			meshContainer->m_userPointer = tmpObject;
			meshContainer->m_sceneNodes.push_back(node);

			if (newMotionState && node)
				newMotionState->addIrrlichtNode(node);
			

		}
	}

	///quick hack to get access to collision objects (for parent/hierarchy rebuild)
	tmpObject->vnode = bulletObject;

	return meshContainer;

}


irr::scene::ISceneNode*	IrrBlendNew::createMeshNode(irr::video::S3DVertex* vertices, int numVertices, unsigned short int* indices, int numIndices,int numTriangles,class btCollisionObject* bulletObject,Blender::Object* tmpObject)
{

	irr::scene::ISceneNode* myNode = 0;

	irr::video::ITexture* texture0=0;
	irr::video::IVideoDriver* driver = m_sceneManager->getVideoDriver();

	Blender::Mesh *me = (Blender::Mesh*)tmpObject->data;
	Blender::Image* image = 0;
	
	bParse::bMain* mainPtr = m_blendFile->getMain();



	if (me->mtface && me->mtface[0].tpage)
	{
		//image = (Blender::Image*)m_blendFile->getMain()->findLibPointer(me->mtface[0].tpage);
		image = me->mtface[0].tpage;
	}

	if (image)
	{
		const char* fileName = image->name;

		texture0 = driver->findTexture(fileName);
		if (!texture0)
		{
			if (image->packedfile)
			{
				void* jpgData = image->packedfile->data;
				int jpgSize = image->packedfile->size;
				if (jpgSize)
				{
					irr::io::IReadFile* file = m_device->getFileSystem()->createMemoryReadFile(jpgData,jpgSize,fileName,false);
					if (file)
					{
						texture0 = driver->getTexture(file);
						printf("width = %d, height = %d\n",texture0->getSize().Width,texture0->getSize().Height);
					}
				}
			}
		}


	}


	if (!texture0)
	{
		static irr::video::ITexture* notFoundTexture = 0;
		if (!notFoundTexture)
		{
			unsigned char*	imageData=new unsigned char[256*256*3];
			for(int y=0;y<256;++y)
			{
				const int	t=y>>4;
				unsigned char*	pi=imageData+y*256*3;
				for(int x=0;x<256;++x)
				{
					const int		s=x>>4;
					const unsigned char	b=180;					
					unsigned char			c=b+((s+t&1)&1)*(255-b);
					pi[0]= 255;
					pi[1]=pi[2]=c;pi+=3;
				}
			}
			irr::core::dimension2d<unsigned int> dim(256,256);
			irr::video::IImage* imag = driver->createImageFromData(irr::video::ECF_R8G8B8,dim,imageData);
			delete imageData;
			notFoundTexture = driver->addTexture("notFound",imag);
			imag->drop();
		}
		
		texture0 = notFoundTexture;
	}

	if (texture0)
	{


#ifdef USE_CUSTOM_NODE
			myNode = new CSampleSceneNode(m_sceneManager->getRootSceneNode(), m_sceneManager, m_guid++,vertices,numVertices,indices,numTriangles,texture0);
#else
			irr::scene::SMesh* mesh = new irr::scene::SMesh();
			irr::scene::SMeshBuffer* meshbuf = new irr::scene::SMeshBuffer();
			meshbuf->append(vertices,numVertices,indices,numTriangles*3);
			mesh->addMeshBuffer(meshbuf);
			irr::scene::SAnimatedMesh* animMesh = new irr::scene::SAnimatedMesh(mesh);
#ifdef USE_VBO
			animMesh->setHardwareMappingHint(scene::EHM_STATIC);
#endif //USE_VBO
			myNode = m_sceneManager->addMeshSceneNode(animMesh);
			myNode->setMaterialTexture(0,texture0);
			myNode->setMaterialFlag(irr::video::EMF_LIGHTING,false);
#endif
		
			// Set rotation
			btVector3 EulerRotation;
			btTransform btr;
			if (bulletObject)
			{
				btr = bulletObject->getWorldTransform();
			} else
			{
				float loc[3] = {tmpObject->loc.x,tmpObject->loc.y,tmpObject->loc.z};
			//	btr.setOrigin(btVector3(loc[IRR_X],loc[IRR_Y],loc[IRR_Z]));
				btr.setOrigin(btVector3(loc[0],loc[1],loc[2]));
				
				btMatrix3x3 rot;
				rot.setEulerZYX(tmpObject->rot.x,tmpObject->rot.y,tmpObject->rot.z);
				btr.setBasis(rot);
			}
			

			btMatrix3x3 bmat33 = btr.getBasis();

			irr::core::matrix4 imat;
			float m[16];
			m[0] = btr.getBasis()[IRR_X][IRR_X];
			m[1] = IRR_X_M*IRR_Y_M*btr.getBasis()[IRR_Y][IRR_X];
			m[2] = IRR_X_M*IRR_Z_M*btr.getBasis()[IRR_Z][IRR_X];
			m[3] = 0;

			m[4] = IRR_X_M*IRR_Y_M*btr.getBasis()[IRR_X][IRR_Y];
			m[5] = btr.getBasis()[IRR_Y][IRR_Y];
			m[6] = IRR_Y_M*IRR_Z_M*btr.getBasis()[IRR_Z][IRR_Y];
			m[7] = 0;

			m[8] = IRR_X_M*IRR_Z_M*btr.getBasis()[IRR_X][IRR_Z];
			m[9] = IRR_Y_M*IRR_Z_M*btr.getBasis()[IRR_Y][IRR_Z];
			m[10] = btr.getBasis()[IRR_Z][IRR_Z];
			m[11] = 0;
			
			m[12] = IRR_X_M*btr.getOrigin()[IRR_X];
			m[13] = IRR_Y_M*btr.getOrigin()[IRR_Y];
			m[14] = IRR_Z_M*btr.getOrigin()[IRR_Z];
			m[15] = 0;

			imat.setM(m);
			
			irr::core::vector3df eulerrot = imat.getRotationDegrees();
			myNode->setRotation(eulerrot);
			myNode->setPosition(irr::core::vector3df(IRR_X_M*btr.getOrigin()[IRR_X],	IRR_Y_M*btr.getOrigin()[IRR_Y],	IRR_Z_M*btr.getOrigin()[IRR_Z]));



			float scaling[3] = {tmpObject->size.x,tmpObject->size.y,tmpObject->size.z};

			myNode->setScale(irr::core::vector3df(scaling[IRR_X],scaling[IRR_Y],scaling[IRR_Z]));
			myNode->getParent()->setPosition(irr::core::vector3df(0,0,0));
			myNode->getParent()->setRotation(irr::core::vector3df(0,0,0));
#ifndef USE_CUSTOM_NODE
			mesh->recalculateBoundingBox();
			animMesh->recalculateBoundingBox();
			animMesh->setDirty();
#endif
		}
	return myNode;

}

void	IrrBlendNew::addCamera(Blender::Object* tmpObject)
{
	irr::scene::ICameraSceneNode* cam = m_sceneManager->getActiveCamera();
	if (cam)
	{
		float pos[3] = {tmpObject->loc.x,tmpObject->loc.y,tmpObject->loc.z};
		cam->setPosition(irr::core::vector3df(pos[IRR_X],pos[IRR_Y],pos[IRR_Z]));
		btMatrix3x3 mat;
		mat.setEulerZYX(tmpObject->rot.x,tmpObject->rot.y,tmpObject->rot.z);
		btVector3 EulerRotation;
		MatrixToEuler(mat, EulerRotation);
		cam->setRotation(irr::core::vector3df(EulerRotation[0], EulerRotation[1], EulerRotation[2]));
		cam->updateAbsolutePosition();
	}
}

void	IrrBlendNew::addLight(Blender::Object* tmpObject)
{

}
