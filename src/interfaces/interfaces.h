#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <vector>
#include "IEngineClient/IEngineClient.h"
#include "CGameEntitySystem/CGameEntitySystem.h"
#include "cs2\entity\C_CSPlayerPawn\C_CSPlayerPawn.h"
#include "cs2\datatypes\cutlbuffer\cutlbuffer.h"
#include "cs2\datatypes\cutlstring\cutlstring.h"
#include "cs2\datatypes\keyvalues\keyvalues.h"
#include "cs2\entity\C_Material\C_Material.h"
#include "../interfaces/Source2/inputsystem.h"
#include "i_entity_system.hpp"
#include "../src/core/i_csgo_input.h"
#include "c_engine_trace.h"
#pragma region sdk_definitons
#define GAME_RESOURCE_SERVICE_CLIENT CS_XOR("GameResourceServiceClientV00")
#define SOURCE2_CLIENT CS_XOR("Source2Client00")
#define SCHEMA_SYSTEM CS_XOR("SchemaSystem_00")
#define INPUT_SYSTEM_VERSION CS_XOR("InputSystemVersion00")
#define SOURCE2_ENGINE_TO_CLIENT CS_XOR("Source2EngineToClient00")
#define ENGINE_CVAR CS_XOR("VEngineCvar00")
#define LOCALIZE CS_XOR("Localize_00")
#define NETWORK_CLIENT_SERVICE CS_XOR("NetworkClientService_00")
#define MATERIAL_SYSTEM2 CS_XOR("VMaterialSystem2_00")
#define RESOURCE_SYSTEM CS_XOR("ResourceSystem013")
#define RESOURCE_HANDLE_UTILS CS_XOR("ResourceHandleUtils001")
#define TICK_INTERVAL 0.015625f
#define TIME_TO_TICKS(TIME) (static_cast<int>(0.5f + static_cast<float>(TIME) / TICK_INTERVAL))
#define TICKS_TO_TIME(TICKS) (TICK_INTERVAL * static_cast<float>(TICKS))
#define ROUND_TO_TICKS(TIME) (TICK_INTERVAL * TIME_TO_TICKS(TIME))
#define TICK_NEVER_THINK (-1)
#pragma endregion
class ISwapChainDx11;
class IMemAlloc;
class CCSGOInput;
class ISchemaSystem;
class IInputSystem;
class IGameResourceService;
class ISource2Client;
class IEngineClient;
class IEngineCVar;
class INetworkClientService;
class IMaterialSystem2;
class IResourceSystem;
class CResourceHandleUtils;
class CPVS;
class CGameTraceManager;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
namespace I
{
	inline void(__fastcall* EnsureCapacityBuffer)(CUtlBuffer*, int) = nullptr;
	inline CUtlBuffer* (__fastcall* ConstructUtlBuffer)(CUtlBuffer*, int, int, int) = nullptr;
	inline void(__fastcall* PutUtlString)(CUtlBuffer*, const char*) = nullptr;
	inline std::int64_t(__fastcall* CreateMaterial)(void*, void*, const char*, void*, unsigned int, unsigned int) = nullptr;
	inline bool(__fastcall* LoadKeyValues)(CKeyValues3*, CUtlString*, CUtlBuffer*, const KV3ID_t*, const char*) = nullptr;
	inline void(__fastcall* ConMsg)(const char*, ...) = nullptr;
	inline void(__fastcall* ConColorMsg)(const Color&, const char*, ...) = nullptr;
	inline IEngineClient* EngineClient = nullptr;
	inline IGameResourceService* GameEntity = nullptr;
	inline ISchemaSystem* SchemaSystem = nullptr;
	inline IInputSystem* InputSys = nullptr;
    inline CGameEntitySystem* EntitySystem = nullptr;
	class Interfaces {
	public:
		bool init();
	};
}
using InstantiateInterfaceFn_t = void* (*)();
class CInterfaceRegister
{
public:
    InstantiateInterfaceFn_t fnCreate;
    const char* szName;
    CInterfaceRegister* pNext;
};
struct IInputSystem;
class i_mem_alloc;
class i_schema_system;
class i_resource_system;
class c_game_particle_manager_system;
class c_particle_manager;
class i_client;
class c_game_trace;
class i_game_resource_service;
class i_resource_handle_utils;
class i_engine_cvar;
class i_global_vars;
class i_engine_client;
class c_game_event_manager;
class c_prediction;
class i_network_client_service;
class ISteamClient;
class ISteamFriends;
class ISteamUtils;
class csgoinput;
class c_base_file_system;
class c_address;
class c_dll {
	HMODULE m_dll;
	size_t m_size;
	char* m_name;
public:
	c_dll() : m_dll(nullptr), m_size(0), m_name(nullptr) {}
	c_dll(const char* name) : m_dll(GetModuleHandleA(name)), m_size(0), m_name(nullptr) {
		if (!m_dll)
			return;
		auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(m_dll);
		auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(m_dll) + dos_header->e_lfanew);
		m_size = nt_header->OptionalHeader.SizeOfImage;
		if (!name)
			return;
		m_name = new char[256];
		strcpy_s(m_name, 256, name);
	}
	c_dll(HMODULE dll) : m_dll(dll) {
		auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(m_dll);
		auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(m_dll) + dos_header->e_lfanew);
		m_size = nt_header->OptionalHeader.SizeOfImage;
		GetModuleFileNameA(m_dll, m_name, sizeof(m_name));
		m_name = strrchr(m_name, '\\') + 1;
	}
	template<typename type_t>
	type_t get_export(const char* name) {
		auto address = GetProcAddress(m_dll, name);
		return *reinterpret_cast<type_t*>(&address);
	}
	uintptr_t get() {
		return reinterpret_cast<uintptr_t>(m_dll);
	}
	const char* get_name() {
		return m_name;
	}
};
class c_address {
	uintptr_t m_address;
public:
	c_address() : m_address(0) {}
	c_address(const uintptr_t address) : m_address(address) {}
	c_address(const void* address) : m_address(reinterpret_cast<uintptr_t>(address)) {}
	operator bool() const
	{
		return m_address != 0;
	}
	template <typename type_t = uintptr_t>
	type_t as() const
	{
		return (type_t)(m_address);
	}
	template <typename type_t = uintptr_t>
	type_t as_fn() const
	{
		return reinterpret_cast<type_t>(m_address);
	}
	template <typename type_t = c_address>
	type_t dereference() const
	{
		return *reinterpret_cast<type_t*>(m_address);
	}
	template <typename type_t = c_address>
	type_t relative() const
	{
		return m_address + *reinterpret_cast<int32_t*>(m_address) + 0x4;
	}
	template <typename type_t = c_address>
	type_t add(size_t offset) const
	{
		return m_address + offset;
	}
	template <typename type_t = c_address>
	type_t sub(size_t offset) const
	{
		return m_address - offset;
	}
	template < typename type_t = c_address >
	type_t abs(std::ptrdiff_t offset = 0x3, std::ptrdiff_t length = 7) const
	{
		std::uintptr_t base = m_address;
		const auto displacement = *reinterpret_cast<std::int32_t*>(base + offset);
		base += displacement;
		base += length;
		return (type_t)(base);
	}
};
class c_interface {
	c_address m_interface;
public:
	c_interface() : m_interface(nullptr) {}
	c_interface(c_address address) : m_interface(address) {}
	c_interface(void* address) : m_interface(address) {}
	operator bool() const
	{
		return m_interface;
	}
	template <typename type_t = uintptr_t>
	type_t as() const
	{
		return m_interface.as<type_t>();
	}
	template <typename ret_t = uintptr_t>
	ret_t get_virtual_function(size_t index)
	{
		return m_interface.dereference().dereference().add(index * sizeof(uintptr_t)).as<ret_t>();
	}
	template <typename ret_t = uintptr_t>
	ret_t get_virtual_function_address(size_t index)
	{
		return get_virtual_function<c_address>(index).dereference().as<ret_t>();
	}
	template <typename ret_t, size_t index, class... args_t>
	ret_t call_virtual_function(args_t... args)
	{
		using fn_t = ret_t(__thiscall*)(void*, args_t...);
		fn_t fn = get_virtual_function<fn_t>(index);
		return fn(m_interface.as<void*>(), args...);
	}
};
#define OFFSET(name, type, offset) \
	type name( ) { return c_address(this).add( offset ).as<type>( ); } \
	type name( ) const { return c_address(this).add( offset ).as<type>( ); }
class c_modules {
private:
	struct modules_t {
		c_dll client_dll{};
		c_dll engine2_dll{};
		c_dll schemasystem_dll{};
		c_dll tier0_dll{};
		c_dll navsystem_dll{};
		c_dll rendersystem_dll{};
		c_dll localize_dll{};
		c_dll scenesystem_dll{};
		c_dll materialsystem2_dll{};
		c_dll resourcesystem_dll{};
		c_dll input_system{};
		c_dll animation_system{};
		c_dll filesystem_stdio{};
		void initialize();
	};
public:
	modules_t m_modules{ };
};
inline const auto g_modules = std::make_unique<c_modules>();
#define INTERVAL_PER_TICK 0.015625f
#define TICK_INTERVAL (INTERVAL_PER_TICK)
#define TIME_TO_TICKS(TIME) (static_cast<int>(0.5f + static_cast<float>(TIME) / TICK_INTERVAL))
#define TICKS_TO_TIME(TICKS) (TICK_INTERVAL * static_cast<float>(TICKS))
class c_interface_register
{
public:
	using create_interface_fn = void* (*)();
	create_interface_fn m_create_interface_fn;
	const char* m_name;
	c_interface_register* m_next;
};
inline const c_interface_register* get_register_list(c_dll* dll)
{
	auto create_interface = dll->get_export<c_address>("CreateInterface");
	if (!create_interface)
		return nullptr;
	return create_interface.add(0x3).relative().dereference().as<c_interface_register*>();
}
template <typename type_t = c_interface>
inline type_t* get_interface(c_dll* dll, const char* name)
{
	const HINSTANCE module_handle = GetModuleHandle(dll->get_name());
	if (!module_handle)
		return nullptr;
	using create_interface_t = type_t * (*)(const char*, int*);
	const create_interface_t create_interface = reinterpret_cast<create_interface_t>(GetProcAddress(module_handle, "CreateInterface"));
	if (!create_interface)
		return nullptr;
	return create_interface(name, nullptr);
};
class c_interfaces
{
public:
	i_entity_system* m_entity_system;
	void* m_input_system;
	i_mem_alloc* m_mem_alloc;
	i_schema_system* m_schema_system;
	i_engine_client* m_engine;
	c_engine_trace* engine_trace;
	IEngineClient* EngineClient = nullptr;
	i_client* m_client;
	i_global_vars* m_global_vars;
	i_network_client_service* m_network_client_service;
	float(__cdecl* m_random_float)(float min, float max) = nullptr;
	int(__cdecl* m_random_seed)(int seed) = nullptr;
	void initialize();
};
inline const auto g_interfaces = std::make_unique<c_interfaces>();
