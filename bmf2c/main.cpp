#include <cstdio>
#include <fstream>
#include "BMFont.h"

int getInt(const std::string &buffer, const std::string &name) {

    unsigned long start, end;

    start = buffer.find(name + "=") + name.length() + 1;
    end = buffer.find(" ", start);

    int ret = atoi(buffer.substr(start, end - start).c_str());
    //printf("%s: %i\n", name.c_str(), ret);

    return ret;
}

int main(int argc, char *argv[]) {

    std::string fnt_path; // = "../impact-25.fnt";

    if (argc != 2) {
        printf("usage: ./bmf2c font.fnt");
        return 0;
    }

    BMFont *bmf = new BMFont();
    fnt_path = argv[1];
    std::ifstream file(fnt_path);
    std::string line;

    while (std::getline(file, line)) {
        if (line.compare(0, 4, "info") == 0) {
            bmf->size = getInt(line, "size");
            bmf->outline = getInt(line, "outline");
        } else if (line.compare(0, 11, "chars count") == 0) {
            bmf->chars_count = getInt(line, "chars count");
        } else if (line.compare(0, 7, "char id") == 0) {
            bmf->chars.push_back({getInt(line, "x"),
                                  getInt(line, "y"),
                                  getInt(line, "width"),
                                  getInt(line, "height"),
                                  getInt(line, "xoffset"),
                                  getInt(line, "yoffset"),
                                  getInt(line, "xadvance")});
        }
    }

    std::ofstream hfile;
    hfile.open(fnt_path + ".h");
    hfile << "typedef struct {\n";
    hfile << "\tint x;\n";
    hfile << "\tint y;\n";
    hfile << "\tint width;\n";
    hfile << "\tint height;\n";
    hfile << "\tint xoffset;\n";
    hfile << "\tint yoffset;\n";
    hfile << "\tint xadvance;\n";
    hfile << "} BMChar;\n\n";

    hfile << "typedef struct {\n";
    hfile << "\tint size;\n";
    hfile << "\tint outline;\n";
    hfile << "\tBMChar chars[" << bmf->chars_count << "];\n";
    hfile << "} BMFont;\n";
    hfile.close();

    std::ofstream cfile;
    cfile.open(fnt_path + ".c");
    cfile << "#include \"" << fnt_path << ".h\"\n\n";
    cfile << "BMFont bmf_font = { " << bmf->size << ", " << bmf->outline << ",\n";
    cfile << "\t\t\t{ \n";
    for (int i = 0; i < bmf->chars.size(); i++) {
        cfile << "\t\t\t\t{ ";
        cfile << bmf->chars[i].x << ", ";
        cfile << bmf->chars[i].y << ", ";
        cfile << bmf->chars[i].width << ", ";
        cfile << bmf->chars[i].height << ", ";
        cfile << bmf->chars[i].xoffset << ", ";
        cfile << bmf->chars[i].yoffset << ", ";
        cfile << bmf->chars[i].xadvance << "},\n";
    }
    cfile << "\t\t\t}\n\t\t};\n";
    cfile.close();

    delete (bmf);

    printf("done\n");

    return 0;
}
