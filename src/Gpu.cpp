// Copyright 2012 Stephen Cakebread

#include "Pch.h"
#include "Common.h"
#include "ShaderPsh.h"
#include "ShaderVsh.h"

extern IDirect3DDevice9* gDevice;

namespace gpu
{

	// VertexBuffer

	struct VertexBuffer
	{
		IDirect3DVertexBuffer9* vb;
	};

	VertexBuffer* CreateVertexBuffer(int elementSize, int elementCount)
	{
		VertexBuffer* vb = new VertexBuffer;

		if (!vb)
			return 0;

		if (FAILED(gDevice->CreateVertexBuffer(elementSize * elementCount, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &vb->vb, 0)))
		{
			delete vb;
			return 0;
		}

		return vb;
	}

	void DestroyVertexBuffer(VertexBuffer* vb)
	{
		if (vb)
		{
			vb->vb->Release();
			delete vb;
		}
	}

	void* Map(VertexBuffer* vb)
	{
		if (vb)
		{
			void* data = 0;

			if (SUCCEEDED(vb->vb->Lock(0, 0, &data, D3DLOCK_DISCARD)))
				return data;
		}

		return 0;
	}

	void Unmap(VertexBuffer* vb)
	{
		if (vb)
			vb->vb->Unlock();
	}

	// ShaderDecl

	struct ShaderDecl
	{
		IDirect3DVertexShader9* vertex;
		IDirect3DPixelShader9* pixel;
		IDirect3DVertexDeclaration9* decl;
	};

	ShaderDecl* CreateShaderDecl(void* vertexShader, int vertexShaderLength, void* pixelShader, int pixelShaderLength)
	{
		ShaderDecl* decl = new ShaderDecl;

		if (!decl)
			return 0;

		if (FAILED(gDevice->CreateVertexShader((DWORD*)vertexShader, &decl->vertex)))
		{
			Panic("CreateVertexShader failed");
		}

		if (FAILED(gDevice->CreatePixelShader((DWORD*)pixelShader, &decl->pixel)))
		{
			Panic("CreatePixelShader failed");
		}

		D3DVERTEXELEMENT9 ve[4] =
		{
			{ 0, (intptr_t)&((Vertex*)0)->pos, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, (intptr_t)&((Vertex*)0)->uv, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			{ 0, (intptr_t)&((Vertex*)0)->colour, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
			D3DDECL_END()
		};

		if (FAILED(gDevice->CreateVertexDeclaration(ve, &decl->decl)))
		{
			Panic("CreateInputLayout failed");
		}

		return decl;
	}

	void DestroyShaderDecl(ShaderDecl* decl)
	{
		if (decl)
		{
			decl->vertex->Release();
			decl->pixel->Release();
			decl->decl->Release();

			delete decl;
		}
	}

	// Texture2d

	struct Texture2d
	{
		IDirect3DTexture9* tex;
	};

	Texture2d* LoadTexture2d(const char* path)
	{
		Texture2d* tex = new Texture2d;

		if (!tex)
			return 0;

		if (FAILED(D3DXCreateTextureFromFileExA(gDevice, path, 0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, 0, 0, &tex->tex)))
		{
			delete tex;
			return 0;
		}

		return tex;
	}

	void DestroyTexture2d(Texture2d* tex)
	{
		if (tex)
		{
			tex->tex->Release();
			delete tex;
		}
	}

	void SetTexture(Texture2d* tex)
	{
		if (tex)
		{
			gDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			gDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

			gDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			gDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			gDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

			gDevice->SetTexture(0, tex->tex);
		}
	}

	// Drawing

	void Init()
	{
	}

	void Clear(const Colour& colour)
	{
		gDevice->Clear(0, 0, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
	}

	void Draw(ShaderDecl* decl, VertexBuffer* vb, int count)
	{
		if (!decl || !vb)
			return;

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		gDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		gDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
		gDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		gDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		gDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		gDevice->SetVertexDeclaration(decl->decl);
		gDevice->SetStreamSource(0, vb->vb, 0, sizeof(Vertex));
		gDevice->SetVertexShader(decl->vertex);
		gDevice->SetPixelShader(decl->pixel);
		gDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, count / 3);
	}

}