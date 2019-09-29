
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

namespace mini
{
    namespace shader_compiler
    {
        struct SourceFile
        {
            std::filesystem::path path;
            std::string strippedSource;

            std::string vsEntryPoint;
            std::string psEntryPoint;
        };

        struct CompiledShader
        {
            ID3DBlob* vsBlob = nullptr;
            ID3DBlob* psBlob = nullptr;

            ID3DBlob* errorBlob = nullptr;
            enum CompilationStatus
            {
                CompileSuccess, CompileError
            } compilationStatus = CompileSuccess;
        };
    }
}

//
int wmain(int argc, wchar_t* argv[])
{
    namespace fs = std::filesystem;
    namespace sc = mini::shader_compiler;

    static const fs::path sourceDir{ "src\\HLSL" };
    static const fs::path targetDir{ "resource\\shaders" };

    if(!fs::exists(sourceDir))
    {
        printf("Can't find source directory: %s\n", sourceDir.u8string().c_str());
        return 1;
    }
    if(!fs::exists(targetDir))
    {
        auto res = fs::create_directory(targetDir);
        if(!res)
        {
            printf("Failed to create target directory: %s\n", targetDir.u8string().c_str());
            return 1;
        }
    }

    std::vector<sc::SourceFile> sourceFiles;
    for(auto const& p : fs::recursive_directory_iterator{ sourceDir })
    {
        if (fs::is_regular_file(p) && p.path().extension() == ".hlsl") {
            printf("Found source file: %s\n", p.path().u8string().c_str());
            sourceFiles.push_back({ p.path() });
        }
    }
    ///

    for(auto& sourceFile : sourceFiles)
    {
        std::ifstream file(sourceFile.path.u8string());
        if(!file.is_open())
        {
            printf("Failed to open source file: %s\n", sourceFile.path.u8string().c_str());
        } else
        {
            // read the file in line by line, look for @ tags, append everything else to shader source
            std::string line;
            while(std::getline(file, line))
            {
                // if the very first character in the line is an @ tag, parse the tag
                // otherwise, just append the line to the shader source @todo strip out unused code?
                if(line[0] == '@')
                {
                    static auto TrimWhitespace = [](std::string const& string)
                    {
                        auto first = string.find_first_not_of(' ');
                        auto last = string.find_last_not_of(' ');
                        return string.substr(first, last - first + 1);
                    };

                    auto const tag = line.substr(1);

                    auto const tagTypeFirst = tag.find_first_not_of(' ');
                    auto const tagTypeLast = tag.find_first_of('{') - 1;
                    auto const tagType = TrimWhitespace(tag.substr(tagTypeFirst, tagTypeLast - tagTypeFirst + 1));

                    auto const tagContentFirst = tagTypeLast + 2;
                    auto const tagContentLast = tag.find_first_of('}') - 1;
                    auto const tagContent = TrimWhitespace(tag.substr(tagContentFirst, tagContentLast - tagContentFirst + 1));

                    if(tagType == "vs_main")
                    {
                        sourceFile.vsEntryPoint = tagContent;
                    }
                    if(tagType == "ps_main")
                    {
                        sourceFile.psEntryPoint = tagContent;
                    }

                } else
                {
                    if (!line.empty() || true) {
                        sourceFile.strippedSource.append(line).append("\n");
                    }
                }
            }
        }
    }

    printf("Compiling %llu source file%s...\n", sourceFiles.size(), sourceFiles.size() != 1 ? "s" : "");

    for(auto const& sourceFile : sourceFiles)
    {
        printf("Compiling %s...\n", sourceFile.path.u8string().c_str());
        sc::CompiledShader shader;
        if(!sourceFile.vsEntryPoint.empty())
        {
            auto res = D3DCompile(sourceFile.strippedSource.c_str(), sourceFile.strippedSource.length(), "Vertex Shader", nullptr, nullptr, sourceFile.vsEntryPoint.c_str(), "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shader.vsBlob, &shader.errorBlob);
            if(FAILED(res))
            {
                printf("Failed to compile vertex shader\n%.*s\n", static_cast<int>(shader.errorBlob->GetBufferSize()), static_cast<char const*>(shader.errorBlob->GetBufferPointer()));
                shader.compilationStatus = sc::CompiledShader::CompileError;
            } else
            {
                printf("Successfully compiled vertex shader\n");
            }
        }
        if(!sourceFile.psEntryPoint.empty())
        {
            auto res = D3DCompile(sourceFile.strippedSource.c_str(), sourceFile.strippedSource.length(), "Vertex Shader", nullptr, nullptr, sourceFile.psEntryPoint.c_str(), "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shader.psBlob, &shader.errorBlob);
            if (FAILED(res))
            {
                printf("Failed to compile pixel shader\n%.*s\n", static_cast<int>(shader.errorBlob->GetBufferSize()), static_cast<char const*>(shader.errorBlob->GetBufferPointer()));
                shader.compilationStatus = sc::CompiledShader::CompileError;
            } else
            {
                printf("Successfully compiled pixel shader\n");
            }
        }
        if(shader.compilationStatus == sc::CompiledShader::CompileError)
        {
            printf("Failed to compile shader\n");
        } else
        {
            auto const relativePath = fs::relative(sourceFile.path, sourceDir);
            auto targetPath = targetDir / relativePath;
            targetPath.replace_extension(".shader");
            printf("Successfully compiled shader, writing to %s...\n", targetPath.u8string().c_str());
            auto targetPathDir = targetPath;
            if(!fs::exists(targetPathDir.remove_filename()))
            {
                fs::create_directory(targetPathDir);
            }
            std::ofstream fout{ targetPath.u8string(), std::ios_base::binary };
            if(fout.is_open())
            {
                fout << static_cast<uint32_t>(targetPath.u8string().size());
                fout << targetPath.u8string().data();
                fout << static_cast<uint32_t>(shader.vsBlob->GetBufferSize()); // vertex shader size
                if(shader.vsBlob != nullptr)
                {
                    fout.write(static_cast<char*>(shader.vsBlob->GetBufferPointer()), shader.vsBlob->GetBufferSize());
                }
                fout << static_cast<uint32_t>(shader.psBlob->GetBufferSize()); // pixel shader size     
                if (shader.psBlob != nullptr)
                {
                    fout.write(static_cast<char*>(shader.psBlob->GetBufferPointer()), shader.psBlob->GetBufferSize());
                }
            }
        }
    }


    ///
    return 0;
}