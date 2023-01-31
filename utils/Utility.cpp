#include "Utility.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
namespace Utility
{
    string readFileContents(string filePath)
    {
        std::ifstream t(filePath);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }

    string replaceStrWith(const std::string srcStr, const std::string dstStr, std::string_view root)
    {
        std::string res = "";
        if (root.length() == 0)
            return res;

        auto pos = root.find(srcStr);
        if (pos != std::string::npos)
        {
            auto p1 = root.substr(0, pos );
            auto p2 = root.substr(pos+srcStr.length(), root.length()- pos + srcStr.length());
            res = std::string(p1) + dstStr + std::string(p2);
        }

        return res;
    }
    
    void savePngFile(std::string filename,int w, int h, int comp, unsigned char *data)
    {
        //int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
        stbi_flip_vertically_on_write(1);
        stbi_write_png("test.png",w,h,comp,data,w*comp);
    }
    
    vector<string> split(string str, string delim)
    {
        if (str.length() <= 1)
            return vector<string>();

        vector<string> resultArray;
        auto found = str.find_first_of(delim);
        decltype(found) prevPos = 0;

        while (found!=std::string::npos)
        {
            resultArray.push_back(str.substr(prevPos, found - prevPos));
            prevPos = found + 1;
            found = str.find_first_of(delim,prevPos);
        }

       


        return resultArray;
    }
    float getReflectionAngle(float cAngle, float normal)
    {
        auto flip = (cAngle+ 180);
        if (flip > 360)flip -= 360;
        //incident angle
        auto iAngle = flip - normal;
        //reflected angle
        auto rAngle = normal - iAngle;
        return rAngle;
    }
}



