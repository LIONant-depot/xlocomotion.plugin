#ifndef XLOCOMOTION_REF_H
#define XLOCOMOTION_REF_H
#pragma once

// How other resources and components name a Locomotion Controller resource
#include "dependencies/xresource_guid/source/xresource_guid.h"

namespace xrsc
{
    inline static constexpr auto    locomotion_controller_type_guid_v   = xresource::type_guid(xresource::guid_generator::Instance64FromString("xLocomotionController"));
    using                           locomotion_controller               = xresource::def_guid<locomotion_controller_type_guid_v>;
}

#endif
