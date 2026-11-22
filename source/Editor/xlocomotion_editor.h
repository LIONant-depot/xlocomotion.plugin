#ifndef XLOCOMOTION_EDITOR_H
#define XLOCOMOTION_EDITOR_H
#pragma once

// Locomotion Controller editor (the Locomotion Editor's first piece): the generic descriptor editor - the Ragdoll it was trained on, the bundle
// folder, the recipe; the compile checks the bundle against the body. Evaluate (play it in the ragdoll world) comes next.
#include "source/Tools/Editor/xeditor_descriptor_editor.h"
#include "../xlocomotion_descriptor.h"
#include "../xlocomotion_xgpu_rsc_loader.h"
#include "../xlocomotion_xgpu_rsc_loader.cpp"        // the resource loader: compiled once, in the host's translation unit (as xragdoll's)

namespace xlocomotion_editor
{
    struct session : xeditor::descriptor_editor
    {
        session(xresource::full_guid Guid, xresource_editor::library::guid LibraryGuid, xgpu::device* pDevice) noexcept
            : descriptor_editor("LocomotionController", Guid, LibraryGuid, pDevice)
        {
            m_Document.Load();
            BindDescriptorInspector();
            AddPanel("Description", dock::left, [this] { m_DescriptorInspector.Show(); });
        }
    };

    inline const xeditor::auto_register_resource_editor g_Registration
    { xrsc::locomotion_controller_type_guid_v
    , [](xresource::full_guid Guid, xresource_editor::library::guid LibraryGuid, xgpu::device* pDevice) -> std::unique_ptr<xeditor::resource_editor>
      { return std::make_unique<session>(Guid, LibraryGuid, pDevice); }
    };
}

#endif
