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



