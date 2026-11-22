#ifndef XLOCOMOTION_XGPU_RSC_LOADER_H
#define XLOCOMOTION_XGPU_RSC_LOADER_H
#pragma once

#include "dependencies/xresource_mgr/source/xresource_mgr.h"
#include "xlocomotion.h"
#include "xlocomotion_ref.h"

// The compiled Locomotion Controller at run time (Play and the editor's ragdoll world): CPU data only, one deserialize
template<>
struct xresource::loader< xrsc::locomotion_controller_type_guid_v >
{
    constexpr static inline auto            type_name_v         = L"LocomotionController";
    constexpr static inline auto            use_death_march_v   = false;
    using                                   data_type           = xlocomotion::controller;

    static data_type*                       Load        (xresource::mgr& Mgr, const full_guid& GUID);
    static void                             Destroy     (xresource::mgr& Mgr, data_type&& Data, const full_guid& GUID);
};

#endif
