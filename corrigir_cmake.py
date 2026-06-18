import os

def fix_cmake_files():
    print("Iniciando varredura nos arquivos de configuração...")
    for root, dirs, files in os.walk("."):
        for file in files:
            if file == "CMakeLists.txt" or file.endswith(".cmake"):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                except Exception as e:
                    continue
                
                if "CXX_MODULE_STD ON" in content:
                    print(f"Encontrado CXX_MODULE_STD em: {filepath}")
                    lines = content.splitlines()
                    new_lines = []
                    for line in lines:
                        if "CXX_MODULE_STD ON" in line and not line.strip().startswith("#"):
                            try:
                                # Tenta extrair o nome do target dinamicamente
                                target_name = line.split("set_target_properties(")[1].split()[0]
                                guard_block = (
                                    f"# Correcao Android NDK\n"
                                    f"if(NOT ANDROID AND TARGET \"__CMAKE::CXX26\")\n"
                                    f"    {line.strip()}\n"
                                    f"else()\n"
                                    f"    message(STATUS \"Ignorando CXX_MODULE_STD para {target_name} no Android\")\n"
                                    f"endif()"
                                )
                                new_lines.append(guard_block)
                            except:
                                new_lines.append(f"# {line} # Desativado para Android")
                        else:
                            new_lines.append(line)
                    
                    with open(filepath, 'w', encoding='utf-8') as f:
                        f.write("\n".join(new_lines))
                    print(f"-> {file} corrigido com sucesso!")

if __name__ == "__main__":
    fix_cmake_files()