#ifndef XLOCOMOTION_DESCRIPTOR_H
#define XLOCOMOTION_DESCRIPTOR_H
#pragma once

// What the user edits and saves for a Locomotion Controller: the trained bundle (a folder of the project, as the trainer exported it) and the
// Ragdoll it was trained on. The compiler checks the bundle against that body and packs it; a mismatch fails the compile with its reason.
#include "dependencies/xresource_pipeline_v2/source/xresource_pipeline.h"
#include "plugins/xragdoll.plugin/source/xragdoll_ref.h"
#include "xlocomotion_ref.h"
#include <string>
#include <vector>

namespace xlocomotion_desc
{
    inline static constexpr auto    resource_type_guid_v    = xrsc::locomotion_controller_type_guid_v;

    struct descriptor : xresource_pipeline::descriptor::base
    {
        using parent = xresource_pipeline::descriptor::base;

        void SetupFromSource(std::string_view FileName) override {}

        void Validate(std::vector<std::string>& Errors) const noexcept override
        {
            if (m_Ragdoll.empty())  Errors.push_back("The ragdoll reference is NULL: a controller is trained on one body");
            if (m_Bundle.empty())   Errors.push_back("The bundle folder is empty: the trainer's export (student.bin, prior.bin, dec.bin, policy.json, stance.txt, gains.txt)");
        }

        xrsc::ragdoll   m_Ragdoll   = {};       // the body it was trained on
        std::wstring    m_Bundle    = {};       // project-relative folder of the trainer's export
        std::string     m_Recipe    = {};       // how it was trained (checkpoint, skill base, knobs): shown, not used

        XPROPERTY_VDEF
        ( "LocomotionController", descriptor
        , obj_member<"Ragdoll", &descriptor::m_Ragdoll, member_help<"The Ragdoll (body) the controller was trained on: the compile checks the bundle against it, and Play refuses another body">>
        , obj_member<"Bundle",  &descriptor::m_Bundle,  member_help<"Project-relative folder of the trainer's export (student.bin, prior.bin, dec.bin, policy.json, stance.txt, gains.txt)">>
        , obj_member<"Recipe",  &descriptor::m_Recipe,  member_help<"How it was trained (checkpoint, skill base, settings): for people, the compile ignores it">>
        )
    };
    XPROPERTY_VREG(descriptor)

    struct factory final : xresource_pipeline::factory_base
    {
        using xresource_pipeline::factory_base::factory_base;

        std::unique_ptr<xresource_pipeline::descriptor::base> CreateDescriptor(void) const noexcept override { return std::make_unique<descriptor>(); }
        xresource::type_guid        ResourceTypeGUID(void) const noexcept override { return resource_type_guid_v; }
        const char*                 ResourceTypeName(void) const noexcept override { return "LocomotionController"; }
        const xproperty::type::object& ResourceXPropertyObject(void) const noexcept override { return *xproperty::getObjectByType<descriptor>(); }
    };

    inline static factory g_Factory{};
}

#endif
