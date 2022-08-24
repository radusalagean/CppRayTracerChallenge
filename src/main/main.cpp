#include <iostream>
#include <sstream>
#include <thread>

#define NOMINMAX
#include <windows.h>
#include <ShlObj.h>
#include <tchar.h>

#include "glad.h"
#include "glfw3.h"

#include <RPly.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "rogll/include.h"

#include <RML.h>

#undef DIFFERENCE
#include <graphics/image.h>
#include <serializer/portable_network_graphics_serializer.h>
#include <renderer/camera.h>
#include <renderer/patterns/checker.h>
#include <renderer/patterns/stripe.h>
#include <math/plane.h>
#include <math/cube.h>
#include <math/plane.h>
#include <helpers/material_helper.h>

// === Learning Resources ===
// Learn OpenGL: https://learnopengl.com/
// The Cherno's OpenGL Playlist: https://youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2
// Documentation: https://docs.gl/
// Open.GL: https://open.gl/

using namespace CppRayTracerChallenge::Core;

static Renderer::Camera* RaytraceCamera;

static RML::Tuple4<float> Red{ 1.0f, 0.0f, 0.0f, 1.0f };
static RML::Tuple4<float> Green{ 0.0f, 1.0f, 0.0f, 1.0f };
static RML::Tuple4<float> Blue{ 0.0f, 0.0f, 1.0f, 1.0f };
static RML::Tuple4<float> White{ 1.0f, 1.0f, 1.0f, 1.0f };
static RML::Tuple4<float> Black{ 0.0f, 0.0f, 0.0f, 0.0f };

static RML::Tuple4<float>* ClearColor = &Black;

static bool MoveDown = false;
static bool MoveUp = false;
static bool MoveLeft = false;
static bool MoveRight = false;
static bool MoveForward = false;
static bool MoveBackward = false;
static bool RotateXClockwise = false;
static bool RotateXCounterClockwise = false;
static bool RotateYClockwise = false;
static bool RotateYCounterClockwise = false;
static bool RotateZClockwise = false;
static bool RotateZCounterClockwise = false;
static bool PerformRender = false;
static float VMove = 0;
static float HMove = 0;
static float DMove = 0;
static float RotX = 0;
static float RotY = 0;
static float RotZ = 0;
static float Fov = 90;

static float CamXRot = 0;
static float CamYRot = 0;

static int RENDER_WIDTH = 1024;
static int RENDER_HEIGHT = 768;

static int WIDTH = 1024;
static int HEIGHT = 768;

static ROGLL::Camera* MainCamera;

enum class EditorObjectType
{
	CUBE,
	PLANE,
	LIGHT
};

struct EditorObject
{
	unsigned int id;
	RML::Transform transform;
	EditorObjectType objectType;
	void* ptrProperties;
};

static unsigned int _GenerateEditorObjectId()
{
	static unsigned int id = 0;
	id++;
	return id;
}

static std::vector<EditorObject> EditorObjects;

static std::atomic<bool> RenderThreadInProgress(false);
std::thread* RenderThread;

std::vector<byte> renderData;
byte* _GetRenderPixels()
{
	if (!RaytraceCamera)
	{
		return renderData.data();
	}

	auto buffer = RaytraceCamera->renderedImage().toBuffer();

	renderData.clear();
	renderData.reserve(buffer.size() * 4);

	for (int colorIndex = 0; colorIndex < buffer.size(); ++colorIndex)
	{
		for (int componentIndex = 0; componentIndex < 4; ++componentIndex)
		{
			byte b = 0x0b;
			Graphics::Color& color = buffer.at(colorIndex);

			if (componentIndex == 0)
			{
				b = static_cast<byte>(std::clamp((int)std::ceil(color.red() * 255), 0, 255));
			}
			else if (componentIndex == 1)
			{
				b = static_cast<byte>(std::clamp((int)std::ceil(color.green() * 255), 0, 255));
			}
			else if (componentIndex == 2)
			{
				b = static_cast<byte>(std::clamp((int)std::ceil(color.blue() * 255), 0, 255));
			}
			else if (componentIndex == 3)
			{
				b = static_cast<byte>(0xFFb); // Alpha 1
			}

			renderData.push_back(b);
		}
	}

	return renderData.data();
}

void _WriteImage(Graphics::Image image, Serializer::BaseImageSerializer& serializer)
{
	std::cout << "Writing Image to Desktop... ";

	serializer.serialize(image);

	const std::vector<unsigned char> ppmBuffer = serializer.buffer();

	TCHAR appData[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL,
		CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE,
		NULL,
		SHGFP_TYPE_CURRENT,
		appData))) {
		std::basic_ostringstream<TCHAR> filePath;

		std::string imageName = std::string("\\generated_image.") + std::string(serializer.fileExtension());

		filePath << appData << _TEXT(imageName.c_str());

		std::ofstream file;
		file.open(filePath.str().c_str(), std::ios_base::binary);

		file.write((const char*)&ppmBuffer[0], ppmBuffer.size());
		file.close();

		std::cout << "Success" << std::endl;
	}
	else
	{
		std::cout << "Failed!" << std::endl;
	}
}

Renderer::World _CreateWorld()
{
	Renderer::World world;

	for (const auto& editorObject : EditorObjects)
	{
		RML::Point pos(editorObject.transform.position.x(), editorObject.transform.position.y(), editorObject.transform.position.z()); // TODO: Convert point to vector in RML

		if (editorObject.objectType == EditorObjectType::LIGHT)
		{
			Renderer::PointLight light(pos, Graphics::Color::white());
			world.addLight(light);
		}
		else if (editorObject.objectType == EditorObjectType::CUBE)
		{
			auto cube = std::make_shared<Math::Cube>();
			Renderer::Shape shape(cube);

			RML::Transform t = editorObject.transform;

			t.scaling = RML::Vector(t.scaling.x() * 0.5, t.scaling.y() * 0.5, t.scaling.z() * 0.5);
			shape.transform(t.matrix());
			world.addObject(shape);
		}
		else if (editorObject.objectType == EditorObjectType::PLANE)
		{
			auto plane = std::make_shared<Math::Plane>();
			Renderer::Shape shape(plane);
			shape.transform(editorObject.transform.matrix());
			world.addObject(shape);
		}
	}

	return world;
}

Graphics::Image _RenderToImage()
{
	std::cout << "Initializing RayTracer... ";
	Renderer::World world = _CreateWorld();

	std::cout << "Configuring Camera... ";

	auto raytraceCameraViewMatrix = Renderer::Camera::viewMatrix(
		MainCamera->transform.position,
		MainCamera->transform.position + MainCamera->transform.rotation.inverse() * RML::Vector::forward(),
		RML::Vector::up()
	);

	RaytraceCamera->transform(raytraceCameraViewMatrix);

	std::cout << "Done" << std::endl;

	std::cout << "Starting Render... ";

	RaytraceCamera->render(world);

	std::cout << "Done" << std::endl;

	return RaytraceCamera->renderedImage();
}

void _RenderWorkerThreadFn()
{
	Graphics::Image image = _RenderToImage();
	Serializer::PortableNetworkGraphicsSerializer serializer;

	_WriteImage(image, serializer);

	RenderThreadInProgress.store(false);

	std::cout << "Render Complete" << std::endl;
}

void _StartFullRender()
{
	std::cout << "Render Started" << std::endl;

	RaytraceCamera = new Renderer::Camera(RENDER_WIDTH, RENDER_HEIGHT, Fov);

	RenderThreadInProgress.store(true);
	RenderThread = new std::thread(_RenderWorkerThreadFn);
}

IMGUI_API bool _DearImGui_BeginStatusBar()
{
	using namespace ImGui;

	ImGuiContext& g = *GImGui;
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)GetMainViewport();

	// Notify of viewport change so GetFrameHeight() can be accurate in case of DPI change
	SetCurrentViewport(NULL, viewport);

	// For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
	// FIXME: This could be generalized as an opt-in way to clamp window->DC.CursorStartPos to avoid SafeArea?
	// FIXME: Consider removing support for safe area down the line... it's messy. Nowadays consoles have support for TV calibration in OS settings.
	g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	float height = GetFrameHeight();
	bool is_open = BeginViewportSideBar("##StatusMenuBar", viewport, ImGuiDir_Down, height, window_flags);
	g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);

	if (is_open)
		BeginMenuBar();
	else
		End();
	return is_open;
}

void _DearImGui_EndStatusBar()
{
	using namespace ImGui;

	EndMenuBar();

	// When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
	// FIXME: With this strategy we won't be able to restore a NULL focus.
	ImGuiContext& g = *GImGui;
	if (g.CurrentWindow == g.NavWindow && g.NavLayer == ImGuiNavLayer_Main && !g.NavAnyRequest)
		FocusTopMostWindowUnderOne(g.NavWindow, NULL);

	End();
}

void _DearImGui_SetNextWindowPosRelative(const ImVec2& vec)
{
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x + vec.x, ImGui::GetMainViewport()->Pos.y + vec.y));
}

static int _PlyVertexCb(p_ply_argument argument)
{
	long attribute;
	std::vector<float>* vertexBuffer;
	ply_get_argument_user_data(argument, (void**)&vertexBuffer, &attribute);

	double data = ply_get_argument_value(argument);

	vertexBuffer->push_back(data);

	return 1;
}

static int faceParseIdx = 0;
static int _PlyFaceCb(p_ply_argument argument)
{
	if (faceParseIdx % 4 == 0)
	{
		faceParseIdx++;
		return 1;
	}

	faceParseIdx++;

	std::vector<unsigned int>* indexBuffer;
	ply_get_argument_user_data(argument, (void**)&indexBuffer, NULL);
	double data = ply_get_argument_value(argument);

	indexBuffer->push_back(data);

	return 1;
}

static void _ProcessInput(const ROGLL::Window& windowRef)
{
	GLFWwindow* window = windowRef.GetHandle();

	MoveDown = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
	MoveUp = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
	MoveLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
	MoveRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
	MoveForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
	MoveBackward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

	RotateXClockwise = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
	RotateXCounterClockwise = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
	RotateYClockwise = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
	RotateYCounterClockwise = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
	RotateZClockwise = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;
	RotateZCounterClockwise = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;

	PerformRender = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;

	VMove = (MoveUp * 1) - (MoveDown * 1);
	HMove = (MoveRight * 1) - (MoveLeft * 1);
	DMove = (MoveForward * 1) - (MoveBackward * 1);

	RotX = (RotateXClockwise * 1) - (RotateXCounterClockwise * 1);
	RotY = (RotateYClockwise * 1) - (RotateYCounterClockwise * 1);
	RotZ = (RotateZClockwise * 1) - (RotateZCounterClockwise * 1);

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		Fov += 0.5;

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		Fov -= 0.5;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void _UpdateCamera(ROGLL::Camera& cam) {
	constexpr double speedFactor = 0.25;

	cam.SetPerspective(WIDTH, HEIGHT, RML::Trig::degrees_to_radians(Fov));
	cam.transform.position += cam.transform.rotation.inverse()
		* RML::Vector(HMove * speedFactor,
			VMove * speedFactor,
			DMove * speedFactor);

	CamXRot += RotX;
	CamYRot += RotY;

	cam.transform.rotation = RML::Quaternion::euler_angles(CamXRot, CamYRot, 0);
}

struct VertexLayout
{
	RML::Tuple3<float> pos;
	RML::Tuple3<float> normal;
};

struct UiTextureVertexLayout
{
	RML::Tuple3<float> pos;
	RML::Tuple2<float> uv;
};

static void _WindowResized(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	WIDTH = width;
	HEIGHT = height;
}

int main(void)
{
	ROGLL::Window window("Reccy's Ray Tracer", WIDTH, HEIGHT);

	glfwSetWindowSizeCallback(window.GetHandle(), _WindowResized);

	RML::Tuple3<float> xyz(-0.5, -0.5, -0.5);
	RML::Tuple3<float> xYz(-0.5, 0.5, -0.5);
	RML::Tuple3<float> Xyz(0.5, -0.5, -0.5);
	RML::Tuple3<float> XYz(0.5, 0.5, -0.5);

	RML::Tuple3<float> xyZ(-0.5, -0.5, 0.5);
	RML::Tuple3<float> xYZ(-0.5, 0.5, 0.5);
	RML::Tuple3<float> XyZ(0.5, -0.5, 0.5);
	RML::Tuple3<float> XYZ(0.5, 0.5, 0.5);

	RML::Tuple3<float> Up(0, 1, 0);
	RML::Tuple3<float> Down(0, -1, 0);
	RML::Tuple3<float> Left(-1, 0, 0);
	RML::Tuple3<float> Right(1, 0, 0);
	RML::Tuple3<float> Forward(0, 0, 1);
	RML::Tuple3<float> Backward(0, 0, -1);

	ROGLL::VertexAttributes layout;
	layout.Add<float>(ROGLL::VertexAttributes::POSITION3, 3);
	layout.Add<float>(ROGLL::VertexAttributes::NORMAL3, 3);

	ROGLL::VertexAttributes gizmoLayout;
	gizmoLayout.Add<float>(ROGLL::VertexAttributes::POSITION3, 3);
	gizmoLayout.Add<float>(ROGLL::VertexAttributes::COLOR3, 3);

	VertexLayout cubeVertsLayout[]
	{
		// FRONT
		{ xyz, Backward },
		{ Xyz, Backward },
		{ XYz, Backward },
		{ xYz, Backward },
		//BACK
		{ xYZ, Forward }, // idx 4
		{ XYZ, Forward },
		{ XyZ, Forward },
		{ xyZ, Forward },
		//TOP
		{ xYz, Up }, // idx 8
		{ XYz, Up },
		{ XYZ, Up },
		{ xYZ, Up },
		//BOTTOM
		{ xyZ, Down }, // idx 12
		{ XyZ, Down },
		{ Xyz, Down },
		{ xyz, Down },
		//LEFT
		{ xyZ, Left }, // idx 16
		{ xyz, Left },
		{ xYz, Left },
		{ xYZ, Left },
		//RIGHT
		{ XyZ, Right }, // idx 20
		{ XYZ, Right },
		{ XYz, Right },
		{ Xyz, Right },
	};

	std::vector<float> cubeVertsFloats;

	for (const auto& vertex : cubeVertsLayout)
	{
		cubeVertsFloats.push_back(vertex.pos.x());
		cubeVertsFloats.push_back(vertex.pos.y());
		cubeVertsFloats.push_back(vertex.pos.z());
		cubeVertsFloats.push_back(vertex.normal.x());
		cubeVertsFloats.push_back(vertex.normal.y());
		cubeVertsFloats.push_back(vertex.normal.z());
	}

	ROGLL::Mesh cubeMesh(
		cubeVertsFloats,
		{
			// FRONT
			0, 1, 2,
			2, 3, 0,
			// BACK
			4, 5, 6,
			6, 7, 4,
			// TOP
			8, 9, 10,
			10, 11, 8,
			// BOTTOM
			12, 13, 14,
			14, 15, 12,
			// LEFT
			16, 17, 18,
			18, 19, 16,
			// RIGHT
			20, 21, 22,
			22, 23, 20,
		},
		layout
		);

	std::vector<float> groundVertFloats
	{
		-100, -0.5, -100, 0, 1, 0,
		 100, -0.5, -100, 0, 1, 0,
		 100, -0.5,  100, 0, 1, 0,
		-100, -0.5,  100, 0, 1, 0
	};

	ROGLL::Mesh groundMesh(
		groundVertFloats,
		{
			0, 1, 2,
			2, 3, 0,
		},
		layout
		);

	ROGLL::MeshInstance groundMeshInstance(groundMesh);
	groundMeshInstance.transform.scale(100, 1, 100);

	// BEGIN GIZMO LOAD
	std::cout << "Loading gizmo3d file... ";

	p_ply gizmoPlyFile = ply_open("res/models/gizmo3d.ply", NULL, 0, NULL);
	if (!gizmoPlyFile || !ply_read_header(gizmoPlyFile))
	{
		std::cout << "ERROR: Could not read gizmo3d file" << std::endl;
		glfwTerminate();
		std::cin.get();
		return -1;
	}

	std::vector<float> gizmoPositions;
	std::vector<float> gizmoNormals;
	std::vector<float> gizmoColors;
	std::vector<unsigned int> gizmoIndices;

	long nvertices, ntriangles;
	nvertices = ply_set_read_cb(gizmoPlyFile, "vertex", "x", _PlyVertexCb, &gizmoPositions, 0);
	ply_set_read_cb(gizmoPlyFile, "vertex", "y", _PlyVertexCb, &gizmoPositions, 1);
	ply_set_read_cb(gizmoPlyFile, "vertex", "z", _PlyVertexCb, &gizmoPositions, 2);
	ply_set_read_cb(gizmoPlyFile, "vertex", "nx", _PlyVertexCb, &gizmoNormals, 3);
	ply_set_read_cb(gizmoPlyFile, "vertex", "ny", _PlyVertexCb, &gizmoNormals, 4);
	ply_set_read_cb(gizmoPlyFile, "vertex", "nz", _PlyVertexCb, &gizmoNormals, 5);
	ply_set_read_cb(gizmoPlyFile, "vertex", "red", _PlyVertexCb, &gizmoColors, 6);
	ply_set_read_cb(gizmoPlyFile, "vertex", "green", _PlyVertexCb, &gizmoColors, 7);
	ply_set_read_cb(gizmoPlyFile, "vertex", "blue", _PlyVertexCb, &gizmoColors, 8);

	ntriangles = ply_set_read_cb(gizmoPlyFile, "face", "vertex_indices", _PlyFaceCb, &gizmoIndices, NULL);

	if (!ply_read(gizmoPlyFile))
	{
		std::cout << "ERROR: Could not read gizmo3d file" << std::endl;
		glfwTerminate();
		std::cin.get();
		return -1;
	}

	ply_close(gizmoPlyFile);

	std::cout << "Successfully loaded gizmo3d file" << std::endl;
	// END GIZMO LOAD

	std::vector<float> gizmoVerts;

	for (long i = 0; i < nvertices; i++)
	{
		float offset = i * 3;
		gizmoVerts.push_back(gizmoPositions.at(offset));
		gizmoVerts.push_back(gizmoPositions.at(offset + 1));
		gizmoVerts.push_back(gizmoPositions.at(offset + 2));

		gizmoVerts.push_back(gizmoColors.at(offset) / 255);
		gizmoVerts.push_back(gizmoColors.at(offset + 1) / 255);
		gizmoVerts.push_back(gizmoColors.at(offset + 2) / 255);
	}

	ROGLL::Mesh gizmoMesh(gizmoVerts, gizmoIndices, gizmoLayout);

	ROGLL::MeshInstance gizmoMeshInstance(gizmoMesh);
	gizmoMeshInstance.transform.scale(0.1, 0.1, 0.1);

	RML::Tuple3<float> xy(-0.5, -0.5, 0);
	RML::Tuple3<float> Xy(0.5, -0.5, 0);
	RML::Tuple3<float> XY(0.5, 0.5, 0);
	RML::Tuple3<float> xY(-0.5, 0.5, 0);

	UiTextureVertexLayout uiTextureVerts[] = {
		xy, { 0, 0 },
		Xy, { 1, 0 },
		XY, { 1, 1 },
		xY, { 0, 1 }
	};

	std::vector<float> uiTextureFloats;

	for (const auto& vert : uiTextureVerts)
	{
		uiTextureFloats.push_back(vert.pos.x());
		uiTextureFloats.push_back(vert.pos.y());
		uiTextureFloats.push_back(vert.pos.z());

		uiTextureFloats.push_back(vert.uv.x());
		uiTextureFloats.push_back(vert.uv.y());
	}

	ROGLL::VertexAttributes uiTextureLayout;
	uiTextureLayout.Add<float>(ROGLL::VertexAttributes::POSITION3, 3);
	uiTextureLayout.Add<float>(ROGLL::VertexAttributes::UV2, 2);

	ROGLL::Mesh uiTextureMesh(
		uiTextureFloats,
		{
			0, 1, 2,
			0, 2, 3
		},
		uiTextureLayout);
	ROGLL::MeshInstance gizmoUiInstance(uiTextureMesh);

	ROGLL::Shader shader("res/shaders/Default.shader");
	ROGLL::Shader gizmoShader("res/shaders/VertexColor.shader");

	ROGLL::Material gizmoMaterial(gizmoShader);
	gizmoMaterial.Set4("objectColor", White);

	ROGLL::Material cubeMaterial(shader);
	cubeMaterial.Set4("objectColor", Blue);

	ROGLL::Material groundMaterial(shader);
	groundMaterial.Set4("objectColor", Green);

	ROGLL::Material lightCubeMaterial(shader);
	lightCubeMaterial.Set4("objectColor", White);

	ROGLL::RenderBatch cubeBatch(&layout, &cubeMaterial);

	ROGLL::RenderBatch gizmoBatch(&gizmoLayout, &gizmoMaterial);
	gizmoBatch.AddInstance(&gizmoMeshInstance);

	ROGLL::RenderBatch groundBatch(&layout, &groundMaterial);
	groundBatch.AddInstance(&groundMeshInstance);

	ROGLL::Camera cam(WIDTH, HEIGHT, 60);
	cam.transform.translate(0, 0, -10); // Initial cam position

	MainCamera = &cam;

	ROGLL::Camera gizmoCam(32, 32, 1); // Psuedo orthograpic projection
	gizmoCam.transform.translate(0, 0, -3.2);

	RML::Tuple3<float> lightPosition{
		1,
		9,
		0
	};

	// IMGUI BEGIN
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window.GetHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
	// IMGUI END

	// New Render Target Begin (For Gizmo)
	GLuint gizmoFramebufferId;
	glGenFramebuffers(1, &gizmoFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, gizmoFramebufferId);

	GLuint gizmoRenderTextureId;
	glGenTextures(1, &gizmoRenderTextureId);
	glBindTexture(GL_TEXTURE_2D, gizmoRenderTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint gizmoRenderBufferId;
	glGenRenderbuffers(1, &gizmoRenderBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, gizmoRenderBufferId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gizmoRenderBufferId);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gizmoRenderTextureId, 0);

	GLenum gizmoDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, gizmoDrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR: Could not create framebuffer" << std::endl;
		glfwTerminate();
		std::cin.get();
		return -1;
	}

	// New Render Target End

	// New Render Target Begin (For Render)
	GLuint raytracerRenderTextureId;
	glGenTextures(1, &raytracerRenderTextureId);
	glBindTexture(GL_TEXTURE_2D, raytracerRenderTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	{
		EditorObject light;
		light.id = _GenerateEditorObjectId();
		light.transform.translate(0, 10, 0);
		light.objectType = EditorObjectType::LIGHT;
		EditorObjects.push_back(light);
	}

	{
		EditorObject cube;
		cube.id = _GenerateEditorObjectId();
		cube.transform.translate(0, 0.5, 0);
		cube.objectType = EditorObjectType::CUBE;
		EditorObjects.push_back(cube);
	}

	{
		EditorObject cube;
		cube.id = _GenerateEditorObjectId();
		cube.transform.translate(1, 1.5, 0);
		cube.objectType = EditorObjectType::CUBE;
		EditorObjects.push_back(cube);
	}

	{
		EditorObject plane;
		plane.id = _GenerateEditorObjectId();
		plane.transform.translate(0, 0, 0);
		plane.objectType = EditorObjectType::PLANE;
		EditorObjects.push_back(plane);
	}

	// New Render Target End

	bool guiShowDearImGuiDemo = false;
	bool guiShowGizmo = true;
	bool guiIsRendering = false;
	bool guiShowRenderSettings = false;
	bool guiShowRenderPreview = false;

	while (!window.ShouldClose())
	{
		_ProcessInput(window);
		_UpdateCamera(cam);

		if (PerformRender && !RenderThreadInProgress.load())
		{
			_StartFullRender();
		}
		
		// UI - Pre Pass
		glBindFramebuffer(GL_FRAMEBUFFER, gizmoFramebufferId);
		glViewport(0, 0, 512, 512);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gizmoMeshInstance.transform.rotation = cam.transform.rotation.inverse();
		gizmoBatch.Render(gizmoCam, lightPosition);

		if (RenderThreadInProgress.load())
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, _GetRenderPixels());
			glBindTexture(GL_TEXTURE_2D, raytracerRenderTextureId);
		}

		// Geometry Pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(ClearColor->x(), ClearColor->y(), ClearColor->z(), ClearColor->w());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Not a great way to setup mesh instances but saves me dealing with memory management for the moment
		std::stack<ROGLL::MeshInstance*> cubeMeshInstances;
		std::stack<ROGLL::MeshInstance*> groundMeshInstances;

		for (const auto& obj : EditorObjects)
		{
			if (obj.objectType == EditorObjectType::CUBE)
			{
				ROGLL::MeshInstance* cubeMeshInstance = new ROGLL::MeshInstance(cubeMesh);
				cubeMeshInstance->transform = obj.transform;
				cubeMeshInstances.push(cubeMeshInstance);
				cubeBatch.AddInstance(cubeMeshInstance);
			}
			else if (obj.objectType == EditorObjectType::PLANE)
			{
				ROGLL::MeshInstance* planeMeshInstance = new ROGLL::MeshInstance(groundMesh);
				planeMeshInstance->transform = obj.transform;
				groundMeshInstances.push(planeMeshInstance);
				groundBatch.AddInstance(planeMeshInstance);
			}
		}

		cubeBatch.Render(cam, lightPosition);
		groundBatch.Render(cam, lightPosition);

		while (!cubeMeshInstances.empty())
		{
			auto ptr = cubeMeshInstances.top();
			cubeBatch.RemoveInstance(ptr);
			delete ptr;
			cubeMeshInstances.pop();
		}

		while (!groundMeshInstances.empty())
		{
			auto ptr = groundMeshInstances.top();
			groundBatch.RemoveInstance(ptr);
			delete ptr;
			groundMeshInstances.pop();
		}

		//IMGUI TEST BEGIN

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		// Render Main Menu Bar
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Exit", "Alt + F4")) {
						glfwSetWindowShouldClose(window.GetHandle(), 1);
					}
					
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Render"))
				{
					ImGui::MenuItem("Settings", nullptr, &guiShowRenderSettings);
					ImGui::MenuItem("Preview", nullptr, &guiShowRenderPreview);
					
					if (ImGui::MenuItem("Start Render", "F5", nullptr, !RenderThreadInProgress.load()))
					{
						_StartFullRender();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Debug"))
				{
					ImGui::MenuItem("Dear ImGui Demo", nullptr, &guiShowDearImGuiDemo);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					ImGui::MenuItem("Gizmo", nullptr, &guiShowGizmo);

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("About"))
				{
					ImGui::MenuItem("Reccy's Ray Tracer", nullptr, nullptr, false);
					ImGui::MenuItem("By Aaron Meaney", nullptr, nullptr, false);
					ImGui::MenuItem("Build date: 18/08/2022", nullptr, nullptr, false); // TODO: Replace with compile time variable
					
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}

		// Render Ray Tracer Settings
		{
			if (guiShowRenderSettings)
			{
				ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;


				ImGui::Begin("Render Settings", nullptr, windowFlags);

				bool renderThreadComplete = RenderThreadInProgress.load();

				if (renderThreadComplete)
				{
					ImGui::Text("Render is in progress. Cannot update settings.");
				}

				ImGui::BeginDisabled(renderThreadComplete);
				int renderDimensions[2] { RENDER_WIDTH, RENDER_HEIGHT };
				ImGui::InputInt2("Render Dimensions", renderDimensions);
				RENDER_WIDTH = renderDimensions[0];
				RENDER_HEIGHT = renderDimensions[1];
				ImGui::EndDisabled();
				ImGui::End();
			}
		}

		// Render Ray Tracer Preview
		{
			if (guiShowRenderPreview)
			{
				ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

				ImGui::Begin("Render Preview", nullptr, windowFlags);
				ImGui::Image((void*)raytracerRenderTextureId, ImVec2(1024, 767));
				ImGui::End();
			}
		}

		// Render Footer Bar
		{
			if (_DearImGui_BeginStatusBar())
			{
				std::stringstream camPosSS;
				camPosSS << "Camera Position: " << cam.transform.position.x() << "x " << cam.transform.position.y() << "y " << cam.transform.position.z() << "z";
				camPosSS << " | ";
				camPosSS << "FOV: " << Fov << " degrees";

				ImGui::MenuItem(camPosSS.str().c_str(), nullptr, nullptr, false);

				_DearImGui_EndStatusBar();
			}
		}

		// Render FPS Window
		{
			if (guiShowDearImGuiDemo)
				ImGui::ShowDemoWindow();
		}

		// Render Gizmo
		{
			if (guiShowGizmo)
			{
				ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;

				_DearImGui_SetNextWindowPosRelative(ImVec2(0, 20));

				ImGui::Begin("Gizmo3D", &guiShowGizmo, windowFlags);
				ImGui::Image((void*)gizmoFramebufferId, ImVec2(128, 128), ImVec2(0,1), ImVec2(1,0));
				ImGui::End();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//IMGUI TEST END

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupCurrentContext);
		}

		window.SwapBuffers();
		window.PollEvents();
	}

	return 0;
}
