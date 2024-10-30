#include "include.h"
#include <chrono>
#include <cstdint>

const static std::unordered_set<std::string> forbidden = {
    "CON",  "PRN",  "AUX",  "NUL",  "COM1", "COM2", "COM3", "COM4",
    "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3",
    "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};

const static std::unordered_set<std::string> supportedExtensions = { // no lst
    "asm", "s", "hla", "inc", "palx", "mid"
};

const static std::string VERSION = "0.1.0";
const static std::string filename;

int main() {
#if defined(_WIN32) || defined(_WIN64)
    // Prepare console
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hConsole, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, dwMode);
#endif
    std::cout << "Enter assembly file/directory (e.g. file.asm or /path/to/file.asm): ";
    std::string filename;
    std::getline(std::cin, filename);
    if (filename.empty() || filename.find_first_not_of(" \t\r\f\v") == std::string::npos)
        dbg::Misc::fexit("Detected empty input");

    // Convert to uppercase for path checking
    std::string ufilename = filename;
    std::transform(ufilename.begin(), ufilename.end(), ufilename.begin(), ::toupper);

    size_t pos = 0;
    while ((pos = ufilename.find('/')) != std::string::npos) {
        std::string part = ufilename.substr(0, pos);
        if (forbidden.count(part) > 0)
            dbg::Misc::fexit("Detected forbidden keyword");
        ufilename.erase(0, pos + 1);
    }

    // Remove the file extension & check whether file type is supported or not
    size_t dotPos = filename.find_last_of('.');
    std::string extension;
    if (dotPos != std::string::npos) {
        extension = filename.substr(dotPos + 1);
        for (char& c : extension) c = static_cast<char>(tolower(c));
        if (supportedExtensions.count(extension) == 0U)
            dbg::Misc::fexit("Detected forbidden keyword");
        ufilename.erase(dotPos);
    }

    // Check the remaining part of the file name
    if (forbidden.count(ufilename) > 0)
        dbg::Misc::fexit("Detected forbidden keyword");

    auto begin = std::chrono::high_resolution_clock::now();

    std::ifstream originalFile(filename);
    if (!originalFile)
        dbg::Misc::fexit("File not found");

    std::string nfilename = filename;
    if (dotPos != std::string::npos) nfilename.insert(dotPos, "_analyzed");
    else nfilename += "_analyzed";

    std::ofstream newFile(nfilename);
    if (!newFile)
        dbg::Misc::fexit("Cannot open " + nfilename + ", exiting.");

    std::string architecture = getArchitecture(filename);

    newFile << "; INFORMATION:" << '\n';
    newFile << "; \tAssembly Analyzer Version: " << VERSION << '\n';
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    struct tm localTime {};
#if defined(_WIN32) || defined (_WIN64)
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif
    newFile << "; \tAnalyzed on: " << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << '\n';
    newFile << "; \tInstruction Set Architecture: " << architecture << '\n' << '\n';

    std::string line;
    while (getline(originalFile, line)) {
        std::string comment = analyzeLine(line);
        newFile << line << (comment.empty() ? "\n" : "\t\t; " + comment + "\n");
    }

    originalFile.close();
    newFile.close();

    auto delta = std::chrono::high_resolution_clock::now() - begin;
    dbg::Macros::info("Successfully analyzed " + filename + " in " + std::to_string(std::chrono::duration<double>(delta).count()) + "s");
    dbg::Misc::prefexit();

    return 0;
}

[[nodiscard]] auto isDirective(const std::string& opcode) -> bool {
    return !opcode.empty() && opcode[0] == '.';
}

[[nodiscard]] auto isMemoryAddressingMode(const std::string& operand) -> bool {
    return !operand.empty() && operand[0] == '[' && operand.back() == ']' && operand.find('%') != std::string::npos;
}

[[nodiscard]] auto getOperand(const std::string& line) -> std::string {
    size_t wPos = line.find(' ');
    if (wPos == std::string::npos) return "";

    std::string operand = line.substr(wPos + 1);
    size_t trailingWPos = operand.find_last_not_of(' ');
    if (trailingWPos != std::string::npos)
        operand = operand.substr(0, trailingWPos + 1);

    return operand;
}

[[nodiscard]] auto analyzeDirective(const std::string& opcode, const std::string& operand) -> std::string {
    if (opcode == ".string") {
        std::string lineValue = trim(operand).substr(8);
        return "string constant " + lineValue + " declared";
    } if (opcode == ".data") {
        return "Data section declared";
    } if (opcode == ".bss") {
        return "BSS (uninitialized data) section declared";
    } if (opcode == ".text") {
        return "Text (code) section declared";
    } if (opcode == ".globl" || opcode == ".global") {
        return "Global symbol " + operand + " declared";
    } if (opcode == ".align") {
        return "Align to " + operand + " bytes";
    } if (opcode == ".byte") {
        return "Byte value " + operand + " declared";
    } if (opcode == ".word") {
        return "Word value " + operand + " declared";
    } if (opcode == ".dword") {
        return "Double word value " + operand + " declared";
    } if (opcode == ".quad") {
        return "Quad word (64-bit) value " + operand + " declared";
    } if (opcode == ".section") {
        return "Section " + operand + " declared";
    } if (opcode == ".equ" || opcode == ".set") {
        return "Constant " + operand + " defined";
    } if (opcode == ".org") {
        return "Set origin to address " + operand;
    } if (opcode == ".reserve" || opcode == ".space") {
        return "Reserve " + operand + " bytes";
    } if (opcode == ".file") {
        return "File name set to " + operand;
    } if (opcode == ".comm") {
        return "Common block " + operand + " declared";
    } if (opcode == ".end") {
        return "End of assembly";
    } if (opcode == ".incbin") {
        return "Include binary file " + operand;
    }

    return "Unknown directive: " + opcode;
}

[[nodiscard]] auto analyzeLine(const std::string& line) -> std::string {
    if (line.find_first_not_of(" \t\r\n") == std::string::npos) return "";

    std::string trimmedLine = trim(line);

    if (!trimmedLine.empty() && trimmedLine[trimmedLine.size() - 1] == ':')
        return "Label: " + trimmedLine.substr(0, trimmedLine.size() - 1);

    size_t spacePos = trimmedLine.find(' ');
    std::string opcode = trimmedLine.substr(0, spacePos);

    if (isInstruction(opcode)) {
        std::string operands = trimmedLine.substr(spacePos + 1);
        return analyzeInstruction(opcode, operands, line);
    }

    if (isDirective(opcode)) {
        std::string operand = getOperand(line);
        return analyzeDirective(opcode, operand);
    }

    return "Unknown instruction";
}

[[nodiscard]] auto analyzeInstruction(const std::string& opcode, const std::string& operands, const std::string& line) -> std::string {
    if (opcode == "global") {
        return "Declare global symbol " + operands;
    } if (opcode == "len") {
        return "Calculate length of " + operands;
    } if (opcode == "int") {
        size_t spacePos = operands.find(' ');
        std::string operand = operands.substr(0, spacePos);

        if (operand.find("0x") == 0) {
            return std::string("Instruction: int ") + std::string("| Interrupt: ") + operand;
        }
    } if (opcode == "push") {
        std::string operand = getOperand(line);
        return "push instruction: pushed " + operand + " into stack";
    } if (opcode == "mov" || opcode == "movq" || opcode == "add" || opcode == "addq" || opcode == "sub" || opcode == "subq") {
        size_t commaPos = operands.find(',');
        if (commaPos != std::string::npos) {
            std::string destination = operands.substr(0, commaPos);
            std::string source = operands.substr(commaPos + 1);
            return "Instruction: " + opcode + " | Destination: "  + \
            analyzeOperand(destination, true) + \
            " | Source:" + analyzeOperand(source, true);
        }
    } if (opcode == "jmp") {
        std::string operand = getOperand(line);
        return "jmp instruction: jumped to " + operand;
    } if (opcode == "call") {
        std::string operand = getOperand(line);
        return "call instruction: called " + operand;
    } if (opcode == "ret") {
        return "ret instruction: returned from function";
    } if (opcode == "nop") {
        return "no operation";
    } if (opcode == "cmp") {
        size_t spacePos = operands.find(' ');
        std::string destOperand = operands.substr(0, spacePos);
        std::string srcOperand = operands.substr(spacePos + 1);

        std::string dest = analyzeOperand(destOperand, true);
        std::string src = analyzeOperand(srcOperand, true);

        return "Instruction: cmp | Destination: " + dest + " | Source: " + src;
    } if (opcode == "je") {
        std::string operand = getOperand(line);
        return "je instruction: jumped to " + operand + " if equal";
    } if (opcode == "jne") {
        std::string operand = getOperand(line);
        return "jne instruction: jumped to " + operand + " if not equal";
    } if (opcode == "inc") {
        std::string operand = getOperand(line);
        return "inc instruction: incremented " + operand;
    } if (opcode == "dec") {
        std::string operand = getOperand(line);
        return "dec instruction: decremented " + operand;
    } if (opcode == "mul") {
        size_t spacePos = operands.find(' ');
        std::string destOperand = operands.substr(0, spacePos);
        std::string srcOperand = operands.substr(spacePos + 1);

        std::string dest = analyzeOperand(destOperand, true);
        std::string src = analyzeOperand(srcOperand, true);

        return "Instruction: mul | Destination: " + dest + " | Source: " + src;
    } if (opcode == "div") {
        size_t spacePos = operands.find(' ');
        std::string destOperand = operands.substr(0, spacePos);
        std::string srcOperand = operands.substr(spacePos + 1);

        std::string dest = analyzeOperand(destOperand, true);
        std::string src = analyzeOperand(srcOperand, true);

        return "Instruction: div | Destination: " + dest + " | Source: " + src;
    }
    return "Unknown instruction: " + opcode;
}

[[nodiscard]] auto analyzeOperands(std::string& operands) -> std::string {
    std::string operandComment;
    std::string operands2 = operands;
    size_t pos = 0;

    while ((pos = operands2.find(' ')) != std::string::npos) {
        std::string operand = operands2.substr(0, pos);
        operandComment += analyzeOperand(operand) + " ";
        operands2.erase(0, pos + 1);
    }

    operandComment += analyzeOperand(operands);
    return operandComment;
}

[[nodiscard]] auto analyzeOperand(std::string& operand, bool appendType) -> std::string {
    if (operand.empty()) return "";

    for (char &c : operand) {
        c = std::tolower(static_cast<uint8_t>(c));
    }

    // Recognized registers
    static const std::unordered_set<std::string> registers = {
        // okay what was i on
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
        "ax", "bx", "cx", "dx", "si", "di", "bp", "sp", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w",
        "al", "ah", "bl", "bh", "cl", "ch", "dl", "dh", "sil", "dil", "bpl", "spl", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b",

        "eip", "rip",
        "eflags", "rflags",

        "st0", "st1", "st2", "st3", "st4", "st5", "st6", "st7",

        "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15",
        "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29", "zmm30", "zmm31",

        //unsed in x86/x64>vvvvv
        "cr0", "cr1", "cr2", "cr3", "cr4",
        "dr0", "dr1", "dr2", "dr3", /* 2 reserved... */ "dr6", "dr7",
        "tr3", "tr4", "tr5", "tr6", "tr7",

        "gdtr", "idtr", "ldtr", "msw",

        // MSRs
        "msr_ia32_apic_base", "msr_ia32_mtrrcap", "msr_ia32_mtrr_physbase0", "msr_ia32_mtrr_physbase1", "msr_ia32_mtrr_physbase2", "msr_ia32_mtrr_physbase3", "msr_ia32_mtrr_physbase4", "msr_ia32_mtrr_physbase5", "msr_ia32_mtrr_physbase6", "msr_ia32_mtrr_physbase7", "msr_ia32_mtrr_physbase8", "msr_ia32_mtrr_physbase9", "msr_ia32_mtrr_physbase10",
        "msr_ia32_mtrr_physmask0", "msr_ia32_mtrr_physmask1", "msr_ia32_mtrr_physmask2", "msr_ia32_mtrr_physmask3", "msr_ia32_mtrr_physmask4", "msr_ia32_mtrr_physmask5", "msr_ia32_mtrr_physmask7", "msr_ia32_mtrr_physmask8", "msr_ia32_mtrr_physmask9", "msr_ia32_mtrr_physmask10", "msr_ia32_perf_status", "msr_ia32_perf_ctl", "msr_ia32_time_stamp_counter",
        "msr_ia32_feature_control", "msr_ia32_sysenter_cs", "msr_ia32_sysenter_esp", "msr_ia32_sysenter_eip", "msr_ia32_debugctl", "msr_ia32_sgxleaf"
    };

    // Check if the operand is a register
    if (registers.count(operand) != 0U)
        return appendType ? operand + " (Register)" : "Register: " + operand;

    // Check for immediate values (numeric literals)
    if (operand[0] == '$' || (std::isdigit(operand[0]) != 0) || operand == "0") {
        std::string immediate = operand[0] == '$' ? operand.substr(1) : operand;
        return appendType ? immediate + " (Immediate)" : "Immediate: " + immediate;
    }

    // Check for memory addressing mode
    if (isMemoryAddressingMode(operand)) {
        std::string labelName = operand.substr(1, operand.size() - 2);
        return appendType ? labelName + " (Memory Address)" : "Memory Address: " + labelName;
    }

    // Assume other operands are labels or identifiers
    return appendType ? operand + " (Label/Identifier)" : "Label/Identifier: " + operand;
}

[[nodiscard]] auto trim(const std::string& str) -> std::string {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) { return str; }

    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

[[nodiscard]] auto isInstruction(const std::string& opcode) -> bool {
    // Recognized instructions
    const static std::vector<std::string> instructions = {
        "int", "push", "pop", "mov", "movq", "add", "addq", "sub", "subq",
        "jmp", "call", "ret", "cmp", "je", "jne", "inc", "dec", "mul", "div",
        "global", "len", "nop"
    };

    return find(instructions.begin(), instructions.end(), opcode) != instructions.end();
}

[[nodiscard]] auto getArchitecture(const std::string& filename) -> std::string {
    std::ifstream file(filename);
    if (!file)
        dbg::Macros::fatal("Error opening/reading " + filename + " file (is " + filename + " closed?)");

    std::string architecture = "Unknown"; // default value is Unknown
    std::string line;

    while (getline(file, line)) {
        line = trim(line);

        // x86-64
        if (line.find(".code64") != std::string::npos || line.find(".x64") != std::string::npos ||
            line.find(".quad") != std::string::npos || line.find("BITS 64") != std::string::npos ||
            line.find("__x86_64__") != std::string::npos || line.find("__amd64__") != std::string::npos ||
            line.find("__aarch64__") != std::string::npos) {
            architecture = "x86-64";
        }
        // x86
        else if (line.find(".code32") != std::string::npos || line.find(".x86") != std::string::npos ||
            line.find("BITS 32") != std::string::npos || line.find("__i386__") != std::string::npos) {
            architecture = "x86";
        }
        // ARM
        else if (line.find(".arm") != std::string::npos || line.find(".thumb") != std::string::npos ||
            line.find("__ARM_ARCH") != std::string::npos || line.find("__arm__") != std::string::npos) {
            architecture = "ARM";
        }
        // MIPS
        else if (line.find(".mips") != std::string::npos || line.find(".mips64") != std::string::npos ||
            line.find("__mips__") != std::string::npos) {
            architecture = "MIPS";
        }
        // PowerPC
        else if (line.find(".ppc") != std::string::npos || line.find("__powerpc__") != std::string::npos ||
            line.find("__ppc__") != std::string::npos) {
            architecture = "PowerPC";
        }
        // RISC-V
        else if (line.find(".riscv") != std::string::npos || line.find("__riscv") != std::string::npos) {
            architecture = "RISC-V";
        }
        // SPARC
        else if (line.find(".sparc") != std::string::npos || line.find("__sparc__") != std::string::npos) {
            architecture = "SPARC";
        }

        if (architecture != "Unknown") { break; }
    }

    file.close();
    return architecture;
}