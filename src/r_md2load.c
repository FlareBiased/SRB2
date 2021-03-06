/*
	From the 'Wizard2' engine by Spaddlewit Inc. ( http://www.spaddlewit.com )
	An experimental work-in-progress.

	Donated to Sonic Team Junior and adapted to work with
	Sonic Robo Blast 2. The license of this code matches whatever
	the licensing is for Sonic Robo Blast 2.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "doomdef.h"
#include "r_md2load.h"
#include "r_model.h"
#include "z_zone.h"

#ifdef SOFTPOLY
#include "polyrenderer/r_softpoly.h"
#endif // SOFTPOLY

#define NUMVERTEXNORMALS 162

// Quake 2 normals are indexed. Use avertexnormals[normalindex][x/y/z] and
// you'll have your normals.
float avertexnormals[NUMVERTEXNORMALS][3] = {
{-0.525731f, 0.000000f, 0.850651f},
{-0.442863f, 0.238856f, 0.864188f},
{-0.295242f, 0.000000f, 0.955423f},
{-0.309017f, 0.500000f, 0.809017f},
{-0.162460f, 0.262866f, 0.951056f},
{0.000000f, 0.000000f, 1.000000f},
{0.000000f, 0.850651f, 0.525731f},
{-0.147621f, 0.716567f, 0.681718f},
{0.147621f, 0.716567f, 0.681718f},
{0.000000f, 0.525731f, 0.850651f},
{0.309017f, 0.500000f, 0.809017f},
{0.525731f, 0.000000f, 0.850651f},
{0.295242f, 0.000000f, 0.955423f},
{0.442863f, 0.238856f, 0.864188f},
{0.162460f, 0.262866f, 0.951056f},
{-0.681718f, 0.147621f, 0.716567f},
{-0.809017f, 0.309017f, 0.500000f},
{-0.587785f, 0.425325f, 0.688191f},
{-0.850651f, 0.525731f, 0.000000f},
{-0.864188f, 0.442863f, 0.238856f},
{-0.716567f, 0.681718f, 0.147621f},
{-0.688191f, 0.587785f, 0.425325f},
{-0.500000f, 0.809017f, 0.309017f},
{-0.238856f, 0.864188f, 0.442863f},
{-0.425325f, 0.688191f, 0.587785f},
{-0.716567f, 0.681718f, -0.147621f},
{-0.500000f, 0.809017f, -0.309017f},
{-0.525731f, 0.850651f, 0.000000f},
{0.000000f, 0.850651f, -0.525731f},
{-0.238856f, 0.864188f, -0.442863f},
{0.000000f, 0.955423f, -0.295242f},
{-0.262866f, 0.951056f, -0.162460f},
{0.000000f, 1.000000f, 0.000000f},
{0.000000f, 0.955423f, 0.295242f},
{-0.262866f, 0.951056f, 0.162460f},
{0.238856f, 0.864188f, 0.442863f},
{0.262866f, 0.951056f, 0.162460f},
{0.500000f, 0.809017f, 0.309017f},
{0.238856f, 0.864188f, -0.442863f},
{0.262866f, 0.951056f, -0.162460f},
{0.500000f, 0.809017f, -0.309017f},
{0.850651f, 0.525731f, 0.000000f},
{0.716567f, 0.681718f, 0.147621f},
{0.716567f, 0.681718f, -0.147621f},
{0.525731f, 0.850651f, 0.000000f},
{0.425325f, 0.688191f, 0.587785f},
{0.864188f, 0.442863f, 0.238856f},
{0.688191f, 0.587785f, 0.425325f},
{0.809017f, 0.309017f, 0.500000f},
{0.681718f, 0.147621f, 0.716567f},
{0.587785f, 0.425325f, 0.688191f},
{0.955423f, 0.295242f, 0.000000f},
{1.000000f, 0.000000f, 0.000000f},
{0.951056f, 0.162460f, 0.262866f},
{0.850651f, -0.525731f, 0.000000f},
{0.955423f, -0.295242f, 0.000000f},
{0.864188f, -0.442863f, 0.238856f},
{0.951056f, -0.162460f, 0.262866f},
{0.809017f, -0.309017f, 0.500000f},
{0.681718f, -0.147621f, 0.716567f},
{0.850651f, 0.000000f, 0.525731f},
{0.864188f, 0.442863f, -0.238856f},
{0.809017f, 0.309017f, -0.500000f},
{0.951056f, 0.162460f, -0.262866f},
{0.525731f, 0.000000f, -0.850651f},
{0.681718f, 0.147621f, -0.716567f},
{0.681718f, -0.147621f, -0.716567f},
{0.850651f, 0.000000f, -0.525731f},
{0.809017f, -0.309017f, -0.500000f},
{0.864188f, -0.442863f, -0.238856f},
{0.951056f, -0.162460f, -0.262866f},
{0.147621f, 0.716567f, -0.681718f},
{0.309017f, 0.500000f, -0.809017f},
{0.425325f, 0.688191f, -0.587785f},
{0.442863f, 0.238856f, -0.864188f},
{0.587785f, 0.425325f, -0.688191f},
{0.688191f, 0.587785f, -0.425325f},
{-0.147621f, 0.716567f, -0.681718f},
{-0.309017f, 0.500000f, -0.809017f},
{0.000000f, 0.525731f, -0.850651f},
{-0.525731f, 0.000000f, -0.850651f},
{-0.442863f, 0.238856f, -0.864188f},
{-0.295242f, 0.000000f, -0.955423f},
{-0.162460f, 0.262866f, -0.951056f},
{0.000000f, 0.000000f, -1.000000f},
{0.295242f, 0.000000f, -0.955423f},
{0.162460f, 0.262866f, -0.951056f},
{-0.442863f, -0.238856f, -0.864188f},
{-0.309017f, -0.500000f, -0.809017f},
{-0.162460f, -0.262866f, -0.951056f},
{0.000000f, -0.850651f, -0.525731f},
{-0.147621f, -0.716567f, -0.681718f},
{0.147621f, -0.716567f, -0.681718f},
{0.000000f, -0.525731f, -0.850651f},
{0.309017f, -0.500000f, -0.809017f},
{0.442863f, -0.238856f, -0.864188f},
{0.162460f, -0.262866f, -0.951056f},
{0.238856f, -0.864188f, -0.442863f},
{0.500000f, -0.809017f, -0.309017f},
{0.425325f, -0.688191f, -0.587785f},
{0.716567f, -0.681718f, -0.147621f},
{0.688191f, -0.587785f, -0.425325f},
{0.587785f, -0.425325f, -0.688191f},
{0.000000f, -0.955423f, -0.295242f},
{0.000000f, -1.000000f, 0.000000f},
{0.262866f, -0.951056f, -0.162460f},
{0.000000f, -0.850651f, 0.525731f},
{0.000000f, -0.955423f, 0.295242f},
{0.238856f, -0.864188f, 0.442863f},
{0.262866f, -0.951056f, 0.162460f},
{0.500000f, -0.809017f, 0.309017f},
{0.716567f, -0.681718f, 0.147621f},
{0.525731f, -0.850651f, 0.000000f},
{-0.238856f, -0.864188f, -0.442863f},
{-0.500000f, -0.809017f, -0.309017f},
{-0.262866f, -0.951056f, -0.162460f},
{-0.850651f, -0.525731f, 0.000000f},
{-0.716567f, -0.681718f, -0.147621f},
{-0.716567f, -0.681718f, 0.147621f},
{-0.525731f, -0.850651f, 0.000000f},
{-0.500000f, -0.809017f, 0.309017f},
{-0.238856f, -0.864188f, 0.442863f},
{-0.262866f, -0.951056f, 0.162460f},
{-0.864188f, -0.442863f, 0.238856f},
{-0.809017f, -0.309017f, 0.500000f},
{-0.688191f, -0.587785f, 0.425325f},
{-0.681718f, -0.147621f, 0.716567f},
{-0.442863f, -0.238856f, 0.864188f},
{-0.587785f, -0.425325f, 0.688191f},
{-0.309017f, -0.500000f, 0.809017f},
{-0.147621f, -0.716567f, 0.681718f},
{-0.425325f, -0.688191f, 0.587785f},
{-0.162460f, -0.262866f, 0.951056f},
{0.442863f, -0.238856f, 0.864188f},
{0.162460f, -0.262866f, 0.951056f},
{0.309017f, -0.500000f, 0.809017f},
{0.147621f, -0.716567f, 0.681718f},
{0.000000f, -0.525731f, 0.850651f},
{0.425325f, -0.688191f, 0.587785f},
{0.587785f, -0.425325f, 0.688191f},
{0.688191f, -0.587785f, 0.425325f},
{-0.955423f, 0.295242f, 0.000000f},
{-0.951056f, 0.162460f, 0.262866f},
{-1.000000f, 0.000000f, 0.000000f},
{-0.850651f, 0.000000f, 0.525731f},
{-0.955423f, -0.295242f, 0.000000f},
{-0.951056f, -0.162460f, 0.262866f},
{-0.864188f, 0.442863f, -0.238856f},
{-0.951056f, 0.162460f, -0.262866f},
{-0.809017f, 0.309017f, -0.500000f},
{-0.864188f, -0.442863f, -0.238856f},
{-0.951056f, -0.162460f, -0.262866f},
{-0.809017f, -0.309017f, -0.500000f},
{-0.681718f, 0.147621f, -0.716567f},
{-0.681718f, -0.147621f, -0.716567f},
{-0.850651f, 0.000000f, -0.525731f},
{-0.688191f, 0.587785f, -0.425325f},
{-0.587785f, 0.425325f, -0.688191f},
{-0.425325f, 0.688191f, -0.587785f},
{-0.425325f, -0.688191f, -0.587785f},
{-0.587785f, -0.425325f, -0.688191f},
{-0.688191f, -0.587785f, -0.425325f},
};

typedef struct
{
	int ident;        // A "magic number" that's used to identify the .md2 file
	int version;      // The version of the file, always 8
	int skinwidth;    // Width of the skin(s) in pixels
	int skinheight;   // Height of the skin(s) in pixels
	int framesize;    // Size of each frame in bytes
	int numSkins;     // Number of skins with the model
	int numXYZ;       // Number of vertices in each frame
	int numST;        // Number of texture coordinates in each frame.
	int numTris;      // Number of triangles in each frame
	int numGLcmds;    // Number of dwords (4 bytes) in the gl command list.
	int numFrames;    // Number of frames
	int offsetSkins;  // Offset, in bytes from the start of the file, to the list of skin names.
	int offsetST;     // Offset, in bytes from the start of the file, to the list of texture coordinates
	int offsetTris;   // Offset, in bytes from the start of the file, to the list of triangles
	int offsetFrames; // Offset, in bytes from the start of the file, to the list of frames
	int offsetGLcmds; // Offset, in bytes from the start of the file, to the list of gl commands
	int offsetEnd;    // Offset, in bytes from the start of the file, to the end of the file (filesize)
} md2header_t;

typedef struct
{
	unsigned short meshIndex[3]; // indices into the array of vertices in each frames
	unsigned short stIndex[3];   // indices into the array of texture coordinates
} md2triangle_t;

typedef struct
{
	short s;
	short t;
} md2texcoord_t;

typedef struct
{
	unsigned char v[3];             // Scaled vertices. You'll need to multiply them with scale[x] to make them normal.
	unsigned char lightNormalIndex; // Index to the array of normals
} md2vertex_t;

typedef struct
{
	float scale[3];      // Used by the v member in the md2framePoint structure
	float translate[3];  // Used by the v member in the md2framePoint structure
	char name[16];       // Name of the frame
} md2frame_t;

// Load the model
model_t *MD2_LoadModelData(UINT8 *buffer, int ztag, boolean useFloat, boolean RSPload)
{
	model_t *retModel = NULL;
	md2header_t *header;

	int i, j;

	const float WUNITS = 1.0f;
	float dataScale = WUNITS;

	md2triangle_t *tris;
	md2texcoord_t *texcoords;
	md2frame_t *frames;
	char *fname = NULL;

	float skw;
	float skh;

	int t;

#ifndef SOFTPOLY
	(void)RSPload;
#endif // SOFTPOLY

	// MD2 currently does not work with tinyframes, so force useFloat = true
	//
	// <SSNTails>
	// the UV coordinates in MD2 are not compatible with glDrawElements like MD3 is. So they need to be loaded as full float.
	//
	// MD2 is intended to be draw in triangle strips and fans
	// not very compatible with a modern GL implementation, either
	// so the idea would be to full float expand it, and put it in a vertex buffer object
	// I'm sure there's a way to convert the UVs to 'tinyframes', but maybe that's a job for someone else.
	// You'd have to decompress the model, then recompress, reindexing the triangles and weeding out duplicate coordinates
	// I already have the decompression work done

	useFloat = true;

	retModel = (model_t*)Z_Calloc(sizeof(model_t), ztag, 0);

	// get pointer to file header
	header = (md2header_t*)buffer;

	retModel->numMeshes = 1; // MD2 only has one mesh
	retModel->meshes = (mesh_t*)Z_Calloc(sizeof(mesh_t) * retModel->numMeshes, ztag, 0);
	retModel->meshes[0].numFrames = header->numFrames;
	// const float WUNITS = 1.0f;
	// float dataScale = WUNITS;

	skw = (float)header->skinwidth;
	skh = (float)header->skinheight;

	// Tris and ST are simple structures that can be straight-copied
	tris = (md2triangle_t*)&buffer[header->offsetTris];
	texcoords = (md2texcoord_t*)&buffer[header->offsetST];
	frames = (md2frame_t*)&buffer[header->offsetFrames];

	retModel->framenames = (char*)Z_Calloc(header->numFrames*16, ztag, 0);
	fname = retModel->framenames;
	for (i = 0; i < header->numFrames; i++)
	{
		memcpy(fname, frames->name, 16);
		fname += 16;
		frames++;
	}

	// Read in textures
	retModel->numMaterials = header->numSkins;

	if (retModel->numMaterials <= 0) // Always at least one skin, duh
		retModel->numMaterials = 1;

	retModel->materials = (material_t*)Z_Calloc(sizeof(material_t)*retModel->numMaterials, ztag, 0);

	// int t;
	for (t = 0; t < retModel->numMaterials; t++)
	{
		retModel->materials[t].ambient[0] = 0.8f;
		retModel->materials[t].ambient[1] = 0.8f;
		retModel->materials[t].ambient[2] = 0.8f;
		retModel->materials[t].ambient[3] = 1.0f;
		retModel->materials[t].diffuse[0] = 0.8f;
		retModel->materials[t].diffuse[1] = 0.8f;
		retModel->materials[t].diffuse[2] = 0.8f;
		retModel->materials[t].diffuse[3] = 1.0f;
		retModel->materials[t].emissive[0] = 0.0f;
		retModel->materials[t].emissive[1] = 0.0f;
		retModel->materials[t].emissive[2] = 0.0f;
		retModel->materials[t].emissive[3] = 1.0f;
		retModel->materials[t].specular[0] = 0.0f;
		retModel->materials[t].specular[1] = 0.0f;
		retModel->materials[t].specular[2] = 0.0f;
		retModel->materials[t].specular[3] = 1.0f;
		retModel->materials[t].shininess = 0.0f;
		retModel->materials[t].spheremap = false;
	}

	retModel->meshes[0].numTriangles = header->numTris;

	if (!useFloat) // Decompress to MD3 'tinyframe' space
	{
		char *ptr;

		md2triangle_t *trisPtr;
		unsigned short *indexptr;
		float *uvptr;

#ifdef SOFTPOLY
        if (RSPload)
			skw = skh = 1.0f;
#endif // SOFTPOLY

		dataScale = 0.015624f; // 1 / 64.0f
		retModel->meshes[0].tinyframes = (tinyframe_t*)Z_Calloc(sizeof(tinyframe_t)*header->numFrames, ztag, 0);
		retModel->meshes[0].numVertices = header->numXYZ;
		retModel->meshes[0].uvs = (float*)Z_Malloc(sizeof(float) * 2 * retModel->meshes[0].numVertices, ztag, 0);
		retModel->meshes[0].indices = (unsigned short*)Z_Malloc(sizeof(unsigned short) * 3 * header->numTris, ztag, 0);

		ptr = (char*)frames;
		for (i = 0; i < header->numFrames; i++, ptr += header->framesize)
		{
			short *vertptr;
			char *normptr;
			// char *tanptr;

			md2vertex_t *vertex;

			md2frame_t *framePtr = (md2frame_t*)ptr;
			retModel->meshes[0].tinyframes[i].vertices = (short*)Z_Malloc(sizeof(short) * 3 * header->numXYZ, ztag, 0);
			retModel->meshes[0].tinyframes[i].normals = (char*)Z_Malloc(sizeof(char) * 3 * header->numXYZ, ztag, 0);

			//			if (retModel->materials[0].lightmap)
			//				retModel->meshes[0].tinyframes[i].tangents = (char*)malloc(sizeof(char));//(char*)Z_Malloc(sizeof(char)*3*header->numVerts, ztag);

			vertptr = retModel->meshes[0].tinyframes[i].vertices;
			normptr = retModel->meshes[0].tinyframes[i].normals;

			//			tanptr = retModel->meshes[0].tinyframes[i].tangents;
			retModel->meshes[0].tinyframes[i].material = &retModel->materials[0];

			framePtr++; // Advance to vertex list
			vertex = (md2vertex_t*)framePtr;
			framePtr--;
			for (j = 0; j < header->numXYZ; j++, vertex++)
			{
				float x = (short)(((vertex->v[0] * framePtr->scale[0]) + framePtr->translate[0]) / dataScale);
				float y = (short)(((vertex->v[2] * framePtr->scale[2]) + framePtr->translate[2]) / dataScale);
				float z = (short)(((vertex->v[1] * framePtr->scale[1]) + framePtr->translate[1]) / dataScale);

#ifdef SOFTPOLY
				// Rotate triangle
				if (RSPload)
				{
					fpvector4_t vec;
					fpquaternion_t quaternion;
					const float model_rotate = 0.707f;

					RSP_MakeVector4(vec, x, y, z);

					quaternion.x = model_rotate;
					quaternion.y = 0;
					quaternion.z = 0;
					quaternion.w = -model_rotate;
					RSP_QuaternionRotateVector(&vec, &quaternion);

					x = vec.x;
					y = vec.y;
					z = vec.z;
				}
				else
#endif // SOFTPOLY
					z *= -1.0f;

				*vertptr = x;
				vertptr++;
				*vertptr = y;
				vertptr++;
				*vertptr = z;
				vertptr++;

				// Normal
				*normptr++ = (char)(avertexnormals[vertex->lightNormalIndex][0] * 127);
				*normptr++ = (char)(avertexnormals[vertex->lightNormalIndex][2] * 127);
				*normptr++ = (char)(avertexnormals[vertex->lightNormalIndex][1] * 127);
			}
		}

		// This doesn't need to be done every frame!
		trisPtr = tris;
		indexptr = retModel->meshes[0].indices;
		uvptr = (float*)retModel->meshes[0].uvs;
		for (j = 0; j < header->numTris; j++, trisPtr++)
		{
			*indexptr = trisPtr->meshIndex[0];
			indexptr++;
			*indexptr = trisPtr->meshIndex[1];
			indexptr++;
			*indexptr = trisPtr->meshIndex[2];
			indexptr++;

			uvptr[trisPtr->meshIndex[0] * 2] = texcoords[trisPtr->stIndex[0]].s / skw;
			uvptr[trisPtr->meshIndex[0] * 2 + 1] = (texcoords[trisPtr->stIndex[0]].t / skh);
			uvptr[trisPtr->meshIndex[1] * 2] = texcoords[trisPtr->stIndex[1]].s / skw;
			uvptr[trisPtr->meshIndex[1] * 2 + 1] = (texcoords[trisPtr->stIndex[1]].t / skh);
			uvptr[trisPtr->meshIndex[2] * 2] = texcoords[trisPtr->stIndex[2]].s / skw;
			uvptr[trisPtr->meshIndex[2] * 2 + 1] = (texcoords[trisPtr->stIndex[2]].t / skh);
		}
	}
	else // Full float loading method
	{
		md2triangle_t *trisPtr;
		float *uvptr;

		char *ptr;

		retModel->meshes[0].numVertices = header->numTris * 3;
		retModel->meshes[0].frames = (mdlframe_t*)Z_Calloc(sizeof(mdlframe_t)*header->numFrames, ztag, 0);
		retModel->meshes[0].uvs = (float*)Z_Malloc(sizeof(float) * 2 * retModel->meshes[0].numVertices, ztag, 0);

		trisPtr = tris;
		uvptr = retModel->meshes[0].uvs;
		for (i = 0; i < retModel->meshes[0].numTriangles; i++, trisPtr++)
		{
			*uvptr++ = texcoords[trisPtr->stIndex[0]].s / skw;
			*uvptr++ = (texcoords[trisPtr->stIndex[0]].t / skh);
			*uvptr++ = texcoords[trisPtr->stIndex[1]].s / skw;
			*uvptr++ = (texcoords[trisPtr->stIndex[1]].t / skh);
			*uvptr++ = texcoords[trisPtr->stIndex[2]].s / skw;
			*uvptr++ = (texcoords[trisPtr->stIndex[2]].t / skh);
		}

		ptr = (char*)frames;
		for (i = 0; i < header->numFrames; i++, ptr += header->framesize)
		{
			float *vertptr, *normptr;

			md2vertex_t *vertex;

			md2frame_t *framePtr = (md2frame_t*)ptr;
			retModel->meshes[0].frames[i].normals = (float*)Z_Malloc(sizeof(float) * 3 * header->numTris * 3, ztag, 0);
			retModel->meshes[0].frames[i].vertices = (float*)Z_Malloc(sizeof(float) * 3 * header->numTris * 3, ztag, 0);
			//			if (retModel->materials[0].lightmap)
			//				retModel->meshes[0].frames[i].tangents = (float*)malloc(sizeof(float));//(float*)Z_Malloc(sizeof(float)*3*header->numTris*3, ztag);
			//float *vertptr, *normptr;
			normptr = (float*)retModel->meshes[0].frames[i].normals;
			vertptr = (float*)retModel->meshes[0].frames[i].vertices;
			trisPtr = tris;

			retModel->meshes[0].frames[i].material = &retModel->materials[0];

			framePtr++; // Advance to vertex list
			vertex = (md2vertex_t*)framePtr;
			framePtr--;
			for (j = 0; j < header->numTris; j++, trisPtr++)
			{
				float x1 = ((vertex[trisPtr->meshIndex[0]].v[0] * framePtr->scale[0]) + framePtr->translate[0]) * WUNITS;
				float y1 = ((vertex[trisPtr->meshIndex[0]].v[2] * framePtr->scale[2]) + framePtr->translate[2]) * WUNITS;
				float z1 = ((vertex[trisPtr->meshIndex[0]].v[1] * framePtr->scale[1]) + framePtr->translate[1]) * WUNITS;

				float x2 = ((vertex[trisPtr->meshIndex[1]].v[0] * framePtr->scale[0]) + framePtr->translate[0]) * WUNITS;
				float y2 = ((vertex[trisPtr->meshIndex[1]].v[2] * framePtr->scale[2]) + framePtr->translate[2]) * WUNITS;
				float z2 = ((vertex[trisPtr->meshIndex[1]].v[1] * framePtr->scale[1]) + framePtr->translate[1]) * WUNITS;

				float x3 = ((vertex[trisPtr->meshIndex[2]].v[0] * framePtr->scale[0]) + framePtr->translate[0]) * WUNITS;
				float y3 = ((vertex[trisPtr->meshIndex[2]].v[2] * framePtr->scale[2]) + framePtr->translate[2]) * WUNITS;
				float z3 = ((vertex[trisPtr->meshIndex[2]].v[1] * framePtr->scale[1]) + framePtr->translate[1]) * WUNITS;

#ifdef SOFTPOLY
				// Rotate triangle
				if (RSPload)
				{
					fpvector4_t vec;
					fpquaternion_t quaternion;
					const float model_rotate = 0.707f;

					#define ROTATE_TRIANGLE(tx, ty, tz) \
						RSP_MakeVector4(vec, tx, ty, tz); \
						quaternion.x = model_rotate; \
						quaternion.y = 0; \
						quaternion.z = 0; \
						quaternion.w = -model_rotate; \
						RSP_QuaternionRotateVector(&vec, &quaternion); \
						tx = vec.x; \
						ty = vec.y; \
						tz = vec.z;

					ROTATE_TRIANGLE(x1, y1, z1)
					ROTATE_TRIANGLE(x2, y2, z2)
					ROTATE_TRIANGLE(x3, y3, z3)

					#undef ROTATE_TRIANGLE
				}
				else
#endif // SOFTPOLY
				{
					z1 *= -1.0f;
					z2 *= -1.0f;
					z3 *= -1.0f;
				}

				*vertptr = x1;
				vertptr++;
				*vertptr = y1;
				vertptr++;
				*vertptr = z1;
				vertptr++;

				*vertptr = x2;
				vertptr++;
				*vertptr = y2;
				vertptr++;
				*vertptr = z2;
				vertptr++;

				*vertptr = x3;
				vertptr++;
				*vertptr = y3;
				vertptr++;
				*vertptr = z3;
				vertptr++;

				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[0]].lightNormalIndex][0];
				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[0]].lightNormalIndex][2];
				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[0]].lightNormalIndex][1];

				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[1]].lightNormalIndex][0];
				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[1]].lightNormalIndex][2];
				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[1]].lightNormalIndex][1];

				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[2]].lightNormalIndex][0];
				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[2]].lightNormalIndex][2];
				*normptr++ = avertexnormals[vertex[trisPtr->meshIndex[2]].lightNormalIndex][1];
			}
		}
	}

	return retModel;
}

model_t *MD2_LoadModelFile(const char *fileName, int ztag, boolean useFloat, boolean RSPload)
{
	FILE *f;

	model_t *retModel = NULL;

	size_t fileLen;

	char *buffer;

	f = fopen(fileName, "rb");

	if (!f)
		return NULL;

	// find length of file
	fseek(f, 0, SEEK_END);
	fileLen = ftell(f);
	fseek(f, 0, SEEK_SET);

	// read in file
	buffer = malloc(fileLen);
	if (fread(buffer, fileLen, 1, f)) { } // squash ignored fread error
	fclose(f);

	retModel = MD2_LoadModelData((UINT8 *)buffer, ztag, useFloat, RSPload);

	free(buffer);
	return retModel;
}
