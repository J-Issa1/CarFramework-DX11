#include "Application.h"
#include <iostream>

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

bool Application::HandleKeyboard(MSG msg)
{
	XMFLOAT3 cameraPosition = _camera->GetPosition();

	switch (msg.wParam)
	{
	case VK_UP:
		_cameraOrbitRadius = max(_cameraOrbitRadiusMin, _cameraOrbitRadius - (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_DOWN:
		_cameraOrbitRadius = min(_cameraOrbitRadiusMax, _cameraOrbitRadius + (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_RIGHT:
		_cameraOrbitAngleXZ -= _cameraSpeed;
		return true;
		break;

	case VK_LEFT:
		_cameraOrbitAngleXZ += _cameraSpeed;
		return true;
		break;
	}

	return false;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;

	DSLessEqual = nullptr;
	RSCullNone = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Load Textures
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Textures\\stone.dds", nullptr, &_pTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Textures\\floor.dds", nullptr, &_pGroundTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Textures\\carTex.dds", nullptr, &_pCarTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Textures\\aiCarTex.dds", nullptr, &_pAICarTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Textures\\grass.dds", nullptr, &_pGrassTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Textures\\Track.dds", nullptr, &_pTrackTextureRV);


	// Setup the scene lighting
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 20.0f;
	basicLight.LightVecW = XMFLOAT3(0.0f, 1.0f, -1.0f);

	Geometry cubeGeometry;
	cubeGeometry.indexBuffer = _pIndexBuffer;
	cubeGeometry.vertexBuffer = _pVertexBuffer;
	cubeGeometry.numberOfIndices = 36;
	cubeGeometry.vertexBufferOffset = 0;
	cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Geometry planeGeometry;
	planeGeometry.indexBuffer = _pPlaneIndexBuffer;
	planeGeometry.vertexBuffer = _pPlaneVertexBuffer;
	planeGeometry.numberOfIndices = 6;
	planeGeometry.vertexBufferOffset = 0;
	planeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Material shinyMaterial;
	shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	shinyMaterial.specularPower = 10.0f;

	Material noSpecMaterial;
	noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	noSpecMaterial.specularPower = 0.0f;

	// ---------------------------------------------------------- Floor ------------------------------------------------------------------------------
	Appearance* floorAppearance = new Appearance(planeGeometry, noSpecMaterial);
	floorAppearance->SetTextureRV(_pGroundTextureRV);
	
	Transform* floorTransform = new Transform();
	floorTransform->SetPosition(0.0f, 0.0f, 0.0f);
	floorTransform->SetScale(15.0f, 15.0f, 15.0f);
	floorTransform->SetRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);

	ParticleModel* particleModel = new ParticleModel(floorTransform);

	floorGameObject = new GameObject("Floor", floorAppearance, floorTransform, particleModel);

	// ---------------------------------------------------------- Track ------------------------------------------------------------------------------
	Geometry trackGeometry = OBJLoader::Load("Resources\\Objects\\RaceTrack.obj", _pd3dDevice, true);

	Appearance* trackAppearance = new Appearance(trackGeometry, noSpecMaterial);
	trackAppearance->SetTextureRV(_pTrackTextureRV);

	Transform* trackTransform = new Transform();
	trackTransform->SetPosition(0.0f, 0.2f, 0.0f);
	trackTransform->SetScale(0.1f, 0.1f, 0.1f);

	ParticleModel* trackParticleModel = new ParticleModel(trackTransform);

	track = new GameObject("RaceTrack", trackAppearance, trackTransform, trackParticleModel);

	// ---------------------------------------------------------- Grass ------------------------------------------------------------------------------
	Geometry grassGeometry = OBJLoader::Load("Resources\\Objects\\RaceTrack.obj", _pd3dDevice, true);

	Appearance* grassAppearance = new Appearance(grassGeometry, noSpecMaterial);
	grassAppearance->SetTextureRV(_pGrassTextureRV);

	Transform* grassTransform = new Transform();
	grassTransform->SetPosition(0.0f, -50.0f, 0.0f);
	grassTransform->SetScale(1.0f, 1.0f, 1.0f);

	ParticleModel* grassParticleModel = new ParticleModel(grassTransform);

	grass = new GameObject("Grass", grassAppearance, grassTransform, grassParticleModel);

	// ---------------------------------------------------------- Cubes ------------------------------------------------------------------------------
	Appearance * crateAppearance = new Appearance(cubeGeometry, shinyMaterial);
	crateAppearance->SetTextureRV(_pTextureRV);

	for (auto i = 0; i < 5; i++)
	{
		Transform * cubeTransform = new Transform();
		cubeTransform->SetScale(0.5f, 0.5f, 0.5f);
		cubeTransform->SetPosition(-4.0f + (i * 2.0f), 0.5f, 10.0f);

		ParticleModel * particleModel = new ParticleModel(cubeTransform);

		GameObject* gameObject = new GameObject("Cube " + i, crateAppearance, cubeTransform, particleModel);

		_gameObjects.push_back(gameObject);
	}

	// Set Cube 1 Velocity
	XMFLOAT3 cubeVel = XMFLOAT3(5.0f, 0.0f, 0.0f);
	_gameObjects[0]->GetParticleModel()->SetVelocity(cubeVel);

	// Set Cube 2 Velocity 
	XMFLOAT3 cubeVel1 = XMFLOAT3(-5.0f, 0.0f, 0.0f);
	_gameObjects[1]->GetParticleModel()->SetVelocity(cubeVel1);

	// ---------------------------------------------------------- Car ------------------------------------------------------------------------------
	Transform * carTransform = new Transform();
	carTransform->SetScale(0.05f, 0.05f, 0.05f);
	carTransform->SetPosition(51.1540947f, 0.8f, -0.812614083f);
	carTransform->SetRotation(0.0f, 179.0f, 0.0f);

	Geometry carGeometry = OBJLoader::Load("Resources\\Objects\\car.obj", _pd3dDevice, true);

	Appearance * carAppearance = new Appearance(carGeometry, shinyMaterial);
	carAppearance->SetTextureRV(_pCarTextureRV);

	CarParticleModel* carPM = new CarParticleModel(carTransform);
	carGameObject = new GameObject("Car", carAppearance, carTransform, carPM);

	carPos = carGameObject->GetTransform()->GetPosition();

	// ---------------------------------------------------------- AI Car ------------------------------------------------------------------------------

	Transform * AICarTransform = new Transform();
	AICarTransform->SetScale(0.05f, 0.05f, 0.05f);
	AICarTransform->SetPosition(68.9606628f,0.8,-0.812614083f);
	AICarTransform->SetRotation(0.0f, 0.0f, 0.0f);

	Geometry AICarGeometry = OBJLoader::Load("Resources\\Objects\\car.obj", _pd3dDevice, true);

	Appearance * AICarAppearance = new Appearance(AICarGeometry, shinyMaterial);
	AICarAppearance->SetTextureRV(_pAICarTextureRV);

	ParticleModel* AIcarPM = new ParticleModel(AICarTransform);
	aiCar = new GameObject("AICar", AICarAppearance, AICarTransform, AIcarPM);

	aiCar->GetParticleModel()->SetCollisionRadius(5.0f);

	// ---------------------------------------------------------- Mountain ------------------------------------------------------------------------------
	Transform * mountainTransform = new Transform();
	mountainTransform->SetPosition(0.0f, 0.0f, 10.0f);
	mountainTransform->SetScale(0.1f, 0.1f, 0.1f);

	Geometry mountainGeometry = OBJLoader::Load("Resources\\Objects\\mountain.obj", _pd3dDevice, true);

	Appearance * mountainAppearance = new Appearance(mountainGeometry, shinyMaterial);
	mountainAppearance->SetTextureRV(_pTextureRV);

	ParticleModel* mountainParticleModel = new ParticleModel(mountainTransform);
	mountainParticleModel->SetVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	mountainParticleModel->SetAcceleration(XMFLOAT3(0.0f, 0.0f, 0.0f));

	mountain = new GameObject("Mountain", mountainAppearance, mountainTransform, mountainParticleModel);

	// ---------------------------------------------------------- Camera ------------------------------------------------------------------------------
	XMFLOAT3 cameraPos = carPM->GetCarForwardVector();
	XMFLOAT3 eye = XMFLOAT3(carPos.x - cameraPos.x, carPos.x - cameraPos.y + 10.0f, carPos.x - cameraPos.z);
	XMFLOAT3 at = XMFLOAT3(carPos.x + cameraPos.x, carPos.y + cameraPos.y, carPos.z + cameraPos.z);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	_camera = new Camera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 1000.0f);

	// Init AI
	wayPointIndex = 0;
	LoadContent();

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;
	
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
    };

    D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;

    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

	// Create vertex buffer
	SimpleVertex planeVertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 5.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(5.0f, 5.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(5.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeVertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneVertexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD indices[] =
    {
		3, 1, 0,
		2, 1, 3,

		6, 4, 5,
		7, 4, 6,

		11, 9, 8,
		10, 9, 11,

		14, 12, 13,
		15, 12, 14,

		19, 17, 16,
		18, 17, 19,

		22, 20, 21,
		23, 20, 22
    };

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;     
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = indices;
    hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;

	// Create plane index buffer
	WORD planeIndices[] =
	{
		0, 3, 1,
		3, 2, 1,
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneIndexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 960, 540};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Semester 2 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	UINT sampleCount = 4;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _renderWidth;
    sd.BufferDesc.Height = _renderHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = sampleCount;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_renderWidth;
    vp.Height = (FLOAT)_renderHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();
	InitIndexBuffer();

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _renderWidth;
	depthStencilDesc.Height = _renderHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = sampleCount;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

	// Rasterizer
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSCullNone);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	_pd3dDevice->CreateDepthStencilState(&dssDesc, &DSLessEqual);

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CWcullMode);

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_pSamplerLinear) _pSamplerLinear->Release();

	if (_pTextureRV) _pTextureRV->Release();
	if (_pGroundTextureRV) _pGroundTextureRV->Release();
	if (_pCarTextureRV) _pCarTextureRV->Release();
	if (_pAICarTextureRV) _pAICarTextureRV->Release();
	if (_pGrassTextureRV) _pGrassTextureRV->Release();
	if (_pTrackTextureRV) _pGrassTextureRV->Release();

    if (_pConstantBuffer) _pConstantBuffer->Release();

    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
	if (_pPlaneVertexBuffer) _pPlaneVertexBuffer->Release();
	if (_pPlaneIndexBuffer) _pPlaneIndexBuffer->Release();

    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();

	if (DSLessEqual) DSLessEqual->Release();
	if (RSCullNone) RSCullNone->Release();

	if (CCWcullMode) CCWcullMode->Release();
	if (CWcullMode) CWcullMode->Release();

	if (_camera)
	{
		delete _camera;
		_camera = nullptr;
	}

	for (auto gameObject : _gameObjects)
	{
		if (gameObject)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}
}

void Application::Update(float t)
{
    // Update our time
    static float timeSinceStart = 0.0f;
    static DWORD dwTimeStart = 0;

    DWORD dwTimeCur = timeGetTime();		//GetTickCount();

    if (dwTimeStart == 0)
        dwTimeStart = dwTimeCur;

	timeSinceStart = (dwTimeCur - dwTimeStart) / 1000.0f;

	// ---------------------------------------------------------- MOVE CAR ------------------------------------------------------------------------------
	float newEngineSpeed = 0.2f;
	bool isAccelerating = false;

	if (GetAsyncKeyState('W'))
	{
		isAccelerating = true;
		CarParticleModel* carPModel = (CarParticleModel*)carGameObject->GetParticleModel();
		carPModel->AddEngineSpeed(newEngineSpeed);
	}
	else if (GetAsyncKeyState('S'))
	{
		isAccelerating = true;
		CarParticleModel* carPModel = (CarParticleModel*)carGameObject->GetParticleModel();
		carPModel->AddEngineSpeed(-newEngineSpeed / 2);
	}
	else
	{
		isAccelerating = false;

		CarParticleModel* carPModel = (CarParticleModel*)carGameObject->GetParticleModel();
		if (carPModel->GetEngineSpeed() > 0)
		{
			carPModel->AddEngineSpeed((-newEngineSpeed * 5));
		}
		else if (carPModel->GetEngineSpeed() < 0)
		{
			carPModel->AddEngineSpeed(newEngineSpeed);
		}
		else if (carPModel->GetEngineSpeed() == 0)
		{
			carPModel->SetEngineSpeed(0.0f);
		}

		if (carPModel->GetEngineSpeed() < newEngineSpeed)
		{
			carPModel->SetAcceleration(XMFLOAT3(0.0f, 0.0f, 0.0f));
		}
		
	}

	CarParticleModel* carPModel = (CarParticleModel*)carGameObject->GetParticleModel();

	if (GetAsyncKeyState('A'))
	{
		float carRotation = carGameObject->GetTransform()->GetRotation().y;
		carRotation -= 0.1f;
		carGameObject->GetTransform()->SetRotation(0.0f, carRotation, 0.0f);
	}
	if (GetAsyncKeyState('D'))
	{
		float carRotation = carGameObject->GetTransform()->GetRotation().y;
		carRotation += 0.1f;
		carGameObject->GetTransform()->SetRotation(0.0f, carRotation, 0.0f);
	}

	//if (carPModel->GetEngineSpeed() == 70.0f)
	//{
	//	carPModel->SetEngineSpeed(70.0f);
	//}
	//
	//if (carPModel->GetRPM() >= 6.0f)
	//{
	//	carPModel->SetRPM(6.0f);
	//}

	// TODO: Fix and clean up Gear system.
	if (GetAsyncKeyState(VK_LSHIFT))
	{
		if (carPModel->GetGear() == 6)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(5);
		}
		else if (carPModel->GetGear() == 5)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(4);
		}
		else if (carPModel->GetGear() == 4)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(3);
		}
		else if (carPModel->GetGear() == 3)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(2);
		}
		else if (carPModel->GetGear() == 2)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(1);
		}
	}
	if (GetAsyncKeyState(VK_LCONTROL))
	{
		if (carPModel->GetGear() == 1)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(2);
		}
		else if (carPModel->GetGear() == 2)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(3);
		}
		else if (carPModel->GetGear() == 3)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(4);
		}
		else if (carPModel->GetGear() == 4)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(5);
		}
		else if (carPModel->GetGear() == 5)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(6);
		}
		else if (carPModel->GetGear() == 6)
		{
			carPModel->SetRPM(0.0f);
			carPModel->SetGear(6);
		}
		if (carPModel->GetGear() == 1)
		{
			carPModel->SetGear(2);

		}
		else if (carPModel->GetGear() == 2)
		{
			carPModel->SetGear(3);
		}
		else if (carPModel->GetGear() == 3)
		{
			carPModel->SetGear(4);
		}
		else if (carPModel->GetGear() == 4)
		{
			carPModel->SetGear(5);
			carPModel->SetEngineSpeed(30.0f);
		}
		else if (carPModel->GetGear() == 5)
		{
			carPModel->SetGear(6);
			carPModel->SetEngineSpeed(20.0f);
		}
	}

	// ---------------------------------------------------- CAMERA --------------------------------------------------------------------------------------------

	XMFLOAT3 carDirection = carPModel->GetCarForwardVector();
	XMFLOAT3 carPosition = carGameObject->GetTransform()->GetPosition();
	XMFLOAT3 cameraPos = XMFLOAT3(carPos.x, carPos.y, carPos.z);

	XMFLOAT3 eye = XMFLOAT3(carPosition.x - (carDirection.x * 25.0f), carPosition.y + 11.0f, carPosition.z - (carDirection.z * 25.0f));
	XMFLOAT3 cameraLookAt = XMFLOAT3(carPosition.x + (carDirection.x * 10.0f), carPosition.y + (carDirection.y * 10.0f), carPosition.z + (carDirection.z * 10.0f));

	// Update Camera
	_camera->SetPosition(eye);
	_camera->SetLookAt(cameraLookAt);
	_camera->Update();

	// Resolve Collision between Cubes and Car
	for (auto gameObject1 : _gameObjects)
	{
		gameObject1->GetParticleModel()->CheckFloorCollision(floorGameObject->GetTransform()->GetPosition());

		if (gameObject1->GetParticleModel()->CheckCollision(carGameObject->GetTransform()->GetPosition(), carGameObject->GetParticleModel()->GetCollisionRadius()) == true)
		{
			gameObject1->GetParticleModel()->ResolveCollision(gameObject1->GetParticleModel(), carGameObject->GetParticleModel());
		}

		// Resolve Collision between Cube and Cube
		for (auto gameObject2 : _gameObjects)
		{
			if (gameObject1 != gameObject2)
			{
				if (gameObject1->GetParticleModel()->CheckCollision(gameObject2->GetTransform()->GetPosition(), gameObject2->GetParticleModel()->GetCollisionRadius()) == true)
				{
					gameObject1->GetParticleModel()->ResolveCollision(gameObject1->GetParticleModel(), gameObject2->GetParticleModel());
				}
			}
		}
	}

	// Update Cube objects
	for (auto gameObject : _gameObjects)
	{
		gameObject->Update(t);
		gameObject->GetParticleModel()->Update(t);
	}

	// Resolve Collision between Car and AICar
	// TODO: Fix Car vs AICar collision, weird glitch happens if they both connect.
	if (carGameObject->GetParticleModel()->CheckCollision(aiCar->GetTransform()->GetPosition(), aiCar->GetParticleModel()->GetCollisionRadius()) == true)
	{
		carGameObject->GetParticleModel()->ResolveCollision(carGameObject->GetParticleModel(), aiCar->GetParticleModel());
	}

	// Update Car
	carGameObject->Update(t);
	carGameObject->GetParticleModel()->Update(t);
	carGameObject->GetParticleModel()->CheckFloorCollision(floorGameObject->GetTransform()->GetPosition());

	// Move AICar
	PathFinding(t);
	aiCar->Update(t);
	aiCar->GetParticleModel()->Update(t);
	aiCar->GetParticleModel()->CheckFloorCollision(floorGameObject->GetTransform()->GetPosition());

	// Update Plane Objects
	track->Update(t);
	mountain->Update(t);
	grass->Update(t);
}

void Application::Draw()
{
    // Clear buffers
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Setup buffers and render scene
	_pImmediateContext->IASetInputLayout(_pVertexLayout);
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

    ConstantBuffer cb;

	XMFLOAT4X4 viewAsFloats = _camera->GetView();
	XMFLOAT4X4 projectionAsFloats = _camera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	cb.light = basicLight;
	cb.EyePosW = _camera->GetPosition();

	// Render all scene objects
	for (auto gameObject : _gameObjects)
	{
		// Get render material
		Appearance * appearance = gameObject->GetAppearance();
		Material material = appearance->GetMaterial();

		// Copy material to shader
		cb.surface.AmbientMtrl = material.ambient;
		cb.surface.DiffuseMtrl = material.diffuse;
		cb.surface.SpecularMtrl = material.specular;

		// Set world matrix
		cb.World = XMMatrixTranspose(gameObject->GetTransform()->GetWorldMatrix());

		// Set texture
		if (appearance->HasTexture())
		{
			ID3D11ShaderResourceView * textureRV = appearance->GetTextureRV();
			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
			cb.HasTexture = 1.0f;
		}
		else
		{
			cb.HasTexture = 0.0f;
		}

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		// Draw object
		gameObject->Draw(_pImmediateContext);
	}

	// -------------------------------------------------------------------------- FLOOR -----------------------------------------------------------

	// Get render material
	Appearance * appearance = floorGameObject->GetAppearance();
	Material material = appearance->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(floorGameObject->GetTransform()->GetWorldMatrix());

	// Set texture
	if (appearance->HasTexture())
	{
		ID3D11ShaderResourceView * textureRV = appearance->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	floorGameObject->Draw(_pImmediateContext);
	floorGameObject->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);

	// -------------------------------------------------------------------------- TRACK -----------------------------------------------------------

	// Get render material
	Appearance * trackAppearance = track->GetAppearance();
	Material trackMaterial = trackAppearance->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = trackMaterial.ambient;
	cb.surface.DiffuseMtrl = trackMaterial.diffuse;
	cb.surface.SpecularMtrl = trackMaterial.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(track->GetTransform()->GetWorldMatrix());

	// Set texture
	if (trackAppearance->HasTexture())
	{
		ID3D11ShaderResourceView * trackTextureRV = trackAppearance->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &trackTextureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	track->Draw(_pImmediateContext);

	// -------------------------------------------------------------------------- GRASS -----------------------------------------------------------

	// Get render material
	Appearance* grassAppearance = grass->GetAppearance();
	Material grassMaterial = grassAppearance->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = grassMaterial.ambient;
	cb.surface.DiffuseMtrl = grassMaterial.diffuse;
	cb.surface.SpecularMtrl = grassMaterial.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(grass->GetTransform()->GetWorldMatrix());

	// Set texture
	if (grassAppearance->HasTexture())
	{
		ID3D11ShaderResourceView* grassTextureRV = grassAppearance->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &grassTextureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	grass->Draw(_pImmediateContext);
	grass->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);

	// -------------------------------------------------------------------------- CAR -----------------------------------------------------------

	Appearance * carAppearance = carGameObject->GetAppearance();
	Material carMaterial = carAppearance->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = carMaterial.ambient;
	cb.surface.DiffuseMtrl = carMaterial.diffuse;
	cb.surface.SpecularMtrl = carMaterial.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(carGameObject->GetTransform()->GetWorldMatrix());

	// Set texture
	if (carAppearance->HasTexture())
	{
		ID3D11ShaderResourceView * _pCarTextureRV = carAppearance->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &_pCarTextureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	carGameObject->Draw(_pImmediateContext);

	// -------------------------------------------------------------------------- AI CAR -----------------------------------------------------------
	Appearance * AICarAppearance = aiCar->GetAppearance();
	Material AICarMaterial = AICarAppearance->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = AICarMaterial.ambient;
	cb.surface.DiffuseMtrl = AICarMaterial.diffuse;
	cb.surface.SpecularMtrl = AICarMaterial.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(aiCar->GetTransform()->GetWorldMatrix());

	// Set texture
	if (AICarAppearance->HasTexture())
	{
		ID3D11ShaderResourceView * _pAICarTextureRV = AICarAppearance->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &_pAICarTextureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	aiCar->Draw(_pImmediateContext);

	// -------------------------------------------------- DRAW MOUNTAIN ----------------------------------------------------------------
	Appearance* mountainAppearance = mountain->GetAppearance();
	Material mountainMaterial = mountainAppearance->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = mountainMaterial.ambient;
	cb.surface.DiffuseMtrl = mountainMaterial.diffuse;
	cb.surface.SpecularMtrl = mountainMaterial.specular;

	// Set world matrix
	cb.World = XMMatrixTranspose(mountain->GetTransform()->GetWorldMatrix());

	// Set texture
	if (mountainAppearance->HasTexture())
	{
		ID3D11ShaderResourceView* mountainTextureRV = mountainAppearance->GetTextureRV();
		_pImmediateContext->PSSetShaderResources(0, 1, &mountainTextureRV);
		cb.HasTexture = 1.0f;
	}
	else
	{
		cb.HasTexture = 0.0f;
	}

	// Update constant buffer
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	// Draw object
	mountain->Draw(_pImmediateContext);

    // Present our back buffer to our front buffer
    _pSwapChain->Present(0, 0);
}

void Application::LoadContent()
{
	ifstream inFile;
	string input;
	inFile.open("AI\\waypoints.txt");

	if (!inFile.good())
	{
		std::cerr << "ERROR: Cannot find waypoints file\n";
		return;
	}
	else
	{
		while (!inFile.eof())
		{
			inFile >> input;
			if (input.compare("w") == 0)
			{
				GetWayPoints(&inFile);
			}
		}
	}
}

void Application::GetWayPoints(ifstream* inFile)
{
	float x, y, z;
	*inFile >> x;
	*inFile >> y;
	*inFile >> z;

	XMFLOAT3 Position = { x, y, z };
	wayPoints.push_back(Position);

}

float Application::CalcDist(XMFLOAT3 position)
{
	float distance = sqrtf(position.x * position.x + position.y * position.y + position.z * position.z);
	return distance;
}

void Application::PathFinding(float t)
{
	float AISpeed = 0.03f;
	XMFLOAT3 currentWaypoint = { 0.0f, 0.0f, 0.0f };

	currentWaypoint = wayPoints[wayPointIndex];

	XMFLOAT3 diff = XMFLOAT3(0.0f, 0.0f, 0.0f);
	diff.x = currentWaypoint.x - aiCar->GetTransform()->GetPosition().x;
	diff.y = currentWaypoint.y - aiCar->GetTransform()->GetPosition().y;
	diff.z = currentWaypoint.z - aiCar->GetTransform()->GetPosition().z;

	float distance = CalcDist(diff);

	// If no waypoints exit method.
	if (wayPoints.empty())
	{
		return;
	}
	else
	{
		currentWaypoint = wayPoints[wayPointIndex];
	}

	// Change to next waypoint.
	if (distance < 10.0f)
	{
		wayPointIndex++;
	}
	// If not inside, it will go around the waypoint in a circle.
	if (wayPointIndex >= wayPoints.size() - 1)
	{
		wayPointIndex = 0;
	}

	diff.x /= distance;
	diff.y /= distance;
	diff.z /= distance;

	XMFLOAT3 newPos = { 0.0f, 0.0f, 0.0f };
	newPos.x += diff.x * AISpeed * t;
	newPos.z += diff.z * AISpeed * t;
	newPos.y = 0.0f;
	aiCar->GetParticleModel()->Move(newPos);

	// Calculate the angle of rotation towards the next waypoint.
	float angle = atan2(currentWaypoint.x - aiCar->GetTransform()->GetPosition().x, currentWaypoint.z - aiCar->GetTransform()->GetPosition().z);
	aiCar->GetTransform()->SetRotation(0.0f, angle, 0.0f);

	AISpeed *= 0.98f;
}

/*void Application::GameLoopDelay(float frameStartTime)
{
	//DWORD timeStep = 16.9;
	//DWORD elapsedTime = 0;

	//frameStartTime = timeGetTime();
	//
	//Update(timeStep);
	//Draw();
	//
	//elapsedTime = timeGetTime() - frameStartTime;
	//
	//if (elapsedTime < timeStep)
	//{
	//	Sleep(timeStep - elapsedTime);
	//}
}*/