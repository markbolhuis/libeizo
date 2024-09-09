import re
import sys

in_f = open(sys.argv[1], "r")
out_f = open(sys.argv[2], "w")

s = in_f.read()

matches = re.findall("EIZO_USAGE_([A-Z0-9_]+) *?= *?0x([0-9a-fA-F]+),", s)

out_f.write("#include \"internal.h\"\n")
out_f.write("\n")
out_f.write("const char *\n")
out_f.write("eizo_usage_to_string(enum eizo_usage usage) {\n")
out_f.write("    switch(usage) {\n")
for match in matches:
    out_f.write("    case EIZO_USAGE_" + match[0] + ":\n")
    out_f.write("        return \"" + match[0] + "\";\n")
out_f.write("    default:\n")
out_f.write("        return nullptr;\n")
out_f.write("    }\n")
out_f.write("}\n")
out_f.write("\n")

