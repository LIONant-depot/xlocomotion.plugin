#ifndef XLOCOMOTION_RUNTIME_H
#define XLOCOMOTION_RUNTIME_H
#pragma once

// The compiled Locomotion Controller (Locomotion Editor design: "a learned follower: weights + contract version + body-profile hash; a compiled,
// versioned RESOURCE"). Two kinds (Kind()): a POSE STUDENT steering a frozen skill base (xanimation::pose_student) or a CLIP TRACKER following an animation (xanimation::clip_tracker). Its files are kept as the trainer
// wrote them (student.bin, prior.bin, dec.bin, policy.json, stance.txt, gains.txt) and read by pose_student::LoadFrom; the body it was trained on
// is named (the Ragdoll resource) with its driven-joint count, checked when compiled and again when bound at run time.
#include "dependencies/xserializer/source/xserializer.h"
#include <cstdint>
#include <cstring>
#include <string>

namespace xlocomotion
{
    struct file
    {
        char            m_Name[32]  = {};
        std::uint32_t   m_iData     = 0, m_nData = 0;       // bytes in controller::m_pData
    };

    struct controller
    {
        inline static constexpr auto xserializer_version_v = 1;
        inline static constexpr std::uint32_t contract_version_v = 4;    // the observation / action contract (v0.4: spherical joints driven, toes on limits)

        inline          controller  (void) noexcept = default;
        inline          controller  (xserializer::stream&) noexcept {}
        inline void     Initialize  (void) noexcept { std::memset(this, 0, sizeof(*this)); }
        inline void     Kill        (void) noexcept { delete[] m_pFiles; delete[] m_pData; Initialize(); }

        // the file's bytes (false: none of that name)
        bool Read(const char* pName, std::string& Out) const noexcept
        {
            for (std::uint32_t i = 0; i < m_nFiles; ++i)
                if (std::strncmp(m_pFiles[i].m_Name, pName, sizeof(file::m_Name)) == 0) { Out.assign(reinterpret_cast<const char*>(m_pData) + m_pFiles[i].m_iData, m_pFiles[i].m_nData); return true; }
            return false;
        }

        // the kind, from its files: a pose student (student.bin, xanimation::pose_student) or a clip tracker (policy.bin, xanimation::clip_tracker)
        enum class kind { POSE_STUDENT, CLIP_TRACKER };
        kind Kind() const noexcept { std::string B; return Read("student.bin", B) ? kind::POSE_STUDENT : kind::CLIP_TRACKER; }

        file*           m_pFiles        {nullptr};
        std::uint8_t*   m_pData         {nullptr};
        std::uint32_t   m_nFiles = 0, m_nData = 0;
        std::uint32_t   m_Contract      = 0;
        std::uint32_t   m_nDriven       = 0;        // driven (spherical) joints of the body it was trained on
        std::uint64_t   m_RagdollInstance = 0;      // the Ragdoll resource of that body
    };
}

namespace xserializer::io_functions
{
    template<> inline xerr SerializeIO<xlocomotion::file>(xserializer::stream& S, const xlocomotion::file& V) noexcept
    {
        xerr Err; false || (Err = S.Serialize(V.m_Name)) || (Err = S.Serialize(V.m_iData)) || (Err = S.Serialize(V.m_nData)); return Err;
    }
    template<> inline xerr SerializeIO<xlocomotion::controller>(xserializer::stream& S, const xlocomotion::controller& C) noexcept
    {
        xerr Err;
        false
            || (Err = S.Serialize(C.m_nFiles)) || (Err = S.Serialize(C.m_pFiles, C.m_nFiles))
            || (Err = S.Serialize(C.m_nData))  || (Err = S.Serialize(C.m_pData, C.m_nData))
            || (Err = S.Serialize(C.m_Contract)) || (Err = S.Serialize(C.m_nDriven)) || (Err = S.Serialize(C.m_RagdollInstance))
            ;
        return Err;
    }
}

#endif
