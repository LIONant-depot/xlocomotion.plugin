#include "xlocomotion_compiler.h"

#include "../xlocomotion_descriptor.h"
#include "../xlocomotion.h"
#include "plugins/xragdoll.plugin/source/xragdoll.h"
#include "dependencies/xAnimation/source/adapters/xanimation_clip_tracker.h"

#include "dependencies/xproperty/source/xcore/my_properties.cpp"
#include "dependencies/xmath/source/bridge/xmath_to_xproperty.h"

#include <filesystem>
#include <format>

namespace xlocomotion_compiler
{
    struct implementation : xlocomotion_compiler::instance
    {
        std::wstring CompiledPath(const wchar_t* pType, std::uint64_t Instance) const
        {
            return std::format(L"{}/WINDOWS/{}/{:02X}/{:02X}/{:X}", m_ProjectPaths.m_ResourcesPlatforms, pType, Instance & 0xff, (Instance >> 8) & 0xff, Instance);
        }

        xerr onCompile(void) noexcept override
        {
            displayProgressBar("Loading Descriptor", 0);
            {
                xproperty::settings::context    Context{};
                auto                            DescriptorFileName = std::format(L"{}/{}/Descriptor.txt", m_ProjectPaths.m_Project, m_InputSrcDescriptorPath);
                if (auto Err = m_Descriptor.Serialize(true, DescriptorFileName, Context); Err)
                    return Err;
            }
            {
                std::vector<std::string> Errors;
                m_Descriptor.Validate(Errors);
                if (not Errors.empty())
                {
                    for (auto& E : Errors) LogMessage(xresource_pipeline::msg_type::ERROR, std::move(E));
                    return xerr::create_f<state, "Validation Errors">();
                }
            }
            displayProgressBar("Loading Descriptor", 1);

            // the body it was trained on: the compiled Ragdoll's driven (spherical) joints
            m_Dependencies.m_Resources.push_back({ m_Descriptor.m_Ragdoll.m_Instance, xrsc::ragdoll_type_guid_v });
            std::uint32_t nDriven = 0;
            {
                xserializer::stream Stream; xragdoll::ragdoll* pRagdoll = nullptr;
                if (auto Err = Stream.Load(CompiledPath(L"Ragdoll", m_Descriptor.m_Ragdoll.m_Instance.m_Value), pRagdoll); Err)
                {
                    LogMessage(xresource_pipeline::msg_type::ERROR, "Could not load the compiled Ragdoll - has it been compiled yet?");
                    return xerr::create_f<state, "No ragdoll">(Err);
                }
                for (std::uint32_t j = 0; j < pRagdoll->m_nJoints; ++j) nDriven += pRagdoll->m_pJoints[j].m_Type == 3;      // (xanimation::solver_joint_type::SPHERICAL)
                xserializer::default_memory_handler_v.Free(xserializer::mem_type{ .m_bUnique = true }, pRagdoll);
            }

            // the bundle: every file read, the student parsed (sizes) and checked against the body
            displayProgressBar("Reading the bundle", 0);
            const auto Dir = std::filesystem::path(m_ProjectPaths.m_Project) / m_Descriptor.m_Bundle;
            // the kind (xlocomotion::controller::Kind): student.bin = a pose student, policy.bin = a clip tracker
            const bool bTracker = !std::filesystem::exists(Dir / "student.bin") && std::filesystem::exists(Dir / "policy.bin");
            std::vector<std::pair<std::string, std::string>> Files;
            auto Take = [&](const auto& Names) { for (const char* pName : Names)
            {
                std::string B;
                if (!xanimation::policy_body::ReadFile((Dir / pName).string(), B)) continue;
                m_Dependencies.m_Assets.push_back((std::filesystem::path(m_Descriptor.m_Bundle) / pName).wstring());
                Files.push_back({ pName, std::move(B) });
            } };
            if (bTracker) Take(xanimation::clip_tracker::bundle_files_v); else Take(xanimation::pose_student::bundle_files_v);
            auto Reader = [&](const char* pName, std::string& Out) { for (auto& [N, B] : Files) if (N == pName) { Out = B; return true; } return false; };
            xanimation::pose_student S; xanimation::clip_tracker T; std::string Why;
            if (!(bTracker ? T.LoadFrom(Reader, Why) : S.LoadFrom(Reader, Why)))
            {
                LogMessage(xresource_pipeline::msg_type::ERROR, std::format("The bundle in {} is incomplete or malformed: {}", Dir.string(), Why));
                return xerr::create_f<state, "Bad bundle">();
            }
            std::array<float, 7> G{};
            if (!(bTracker ? T.Gains(0, G) : S.Gains(0, G))) { LogMessage(xresource_pipeline::msg_type::ERROR, "The bundle has no gains.txt: the drives it was trained with are needed in Play"); return xerr::create_f<state, "No gains">(); }
            const std::size_t nBundle = bTracker ? [&] { std::size_t n = 0; while (T.Gains(n, G)) ++n; return n; }() : S.StanceSize();
            if (std::uint32_t(nBundle) != nDriven)
            {
                LogMessage(xresource_pipeline::msg_type::ERROR, std::format("The bundle drives {} joints, the Ragdoll has {} driven (spherical) joints: it was trained on another body", nBundle, nDriven));
                return xerr::create_f<state, "Body mismatch">();
            }
            LogMessage(xresource_pipeline::msg_type::INFO, std::format("{}: {} driven joints, {} files, contract v{}", bTracker ? "clip tracker" : "pose student", nDriven, Files.size(), xlocomotion::controller::contract_version_v));
            displayProgressBar("Reading the bundle", 1);

            // pack
            m_Final.Kill();
            m_Final.m_nFiles = std::uint32_t(Files.size()); m_Final.m_pFiles = new xlocomotion::file[m_Final.m_nFiles];
            for (auto& [N, B] : Files) m_Final.m_nData += std::uint32_t(B.size());
            m_Final.m_pData = new std::uint8_t[m_Final.m_nData];
            std::uint32_t At = 0;
            for (std::uint32_t i = 0; i < m_Final.m_nFiles; ++i)
            {
                auto& F = m_Final.m_pFiles[i]; F = {}; std::strncpy(F.m_Name, Files[i].first.c_str(), sizeof(F.m_Name) - 1);
                F.m_iData = At; F.m_nData = std::uint32_t(Files[i].second.size()); std::memcpy(m_Final.m_pData + At, Files[i].second.data(), F.m_nData); At += F.m_nData;
            }
            m_Final.m_Contract = xlocomotion::controller::contract_version_v; m_Final.m_nDriven = nDriven; m_Final.m_RagdollInstance = m_Descriptor.m_Ragdoll.m_Instance.m_Value;

            for (auto& T : m_Target)
            {
                if (!T.m_bValid) continue;
                xserializer::stream Serializer;
                if (auto Err = Serializer.Save(T.m_DataPath, m_Final
                    , m_OptimizationType == optimization_type::O0 ? xserializer::compression_level::FAST : m_OptimizationType == optimization_type::O1 ? xserializer::compression_level::MEDIUM : xserializer::compression_level::HIGH); Err)
                    return xerr::create_f<state, "Failed to save the compiled controller">(Err);
            }
            return {};
        }

        implementation() { m_Final.Initialize(); }
        ~implementation() { m_Final.Kill(); }

        xlocomotion_desc::descriptor    m_Descriptor;
        xlocomotion::controller         m_Final;
    };

    std::unique_ptr<instance> instance::Create(void)
    {
        return std::make_unique<implementation>();
    }
}
