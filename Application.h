#pragma once

#define CUBECOUNT 2

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "GameObject.h"
#include "ParticleModel.h"
#include "CarParticleModel.h"
#include "resource.h"
#include "Camera.h"
#include "Structures.h"
#include "OBJLoader.h"

#include <vector>

using namespace DirectX;

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;

	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;

	ID3D11Buffer*           _pPlaneVertexBuffer;
	ID3D11Buffer*           _pPlaneIndexBuffer;

	ID3D11Buffer*           _pConstantBuffer;

	ID3D11DepthStencilView* _depthStencilView = nullptr;
	ID3D11Texture2D* _depthStencilBuffer = nullptr;

	ID3D11ShaderResourceView * _pTextureRV = nullptr;
	ID3D11ShaderResourceView * _pGroundTextureRV = nullptr;
	ID3D11ShaderResourceView * _pGrassTextureRV = nullptr;
	ID3D11ShaderResourceView * _pTrackTextureRV = nullptr;
	ID3D11ShaderResourceView * _pCarTextureRV = nullptr;
	ID3D11ShaderResourceView * _pAICarTextureRV = nullptr;

	ID3D11SamplerState * _pSamplerLinear = nullptr;

	Light basicLight;

	vector<GameObject*> _gameObjects;
	GameObject* floorGameObject;
	GameObject* grass;
	GameObject* track;
	GameObject* mountain;
	GameObject* carGameObject;
	GameObject* aiCar;

	XMFLOAT3 carPos;

	Camera * _camera;

	float _cameraOrbitRadius = 7.0f;
	float _cameraOrbitRadiusMin = 2.0f;
	float _cameraOrbitRadiusMax = 50.0f;
	float _cameraOrbitAngleXZ = -90.0f;
	float _cameraSpeed = 2.0f;

	UINT _WindowHeight;
	UINT _WindowWidth;

	// Render dimensions - Change here to alter screen resolution
	UINT _renderHeight = 1080;
	UINT _renderWidth = 1920;

	ID3D11DepthStencilState* DSLessEqual;
	ID3D11RasterizerState* RSCullNone;

	ID3D11RasterizerState* CCWcullMode;
	ID3D11RasterizerState* CWcullMode;

	void LoadContent();
	void PathFinding(float t);
	float CalcDist(XMFLOAT3 position);
	void GetWayPoints(ifstream* inFIle);

	vector <XMFLOAT3> wayPoints;
	int wayPointIndex = 0;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg);

	void Update(float t);
	void Draw();

	void GameLoopDelay(float frameStartTime);
};

