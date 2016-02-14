#pragma once

namespace MuduoPlus
{
    class NonCopyable
    {
    protected:
        NonCopyable(){}
        ~NonCopyable(){}

    private:
        NonCopyable     (const NonCopyable&);
        NonCopyable&    operator=(const NonCopyable&);        
    };
}