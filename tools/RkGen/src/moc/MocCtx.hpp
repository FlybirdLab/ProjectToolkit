#pragma once

namespace toolkit {

    class MocCtx
    {
    public:
        MocCtx(int argc, char** argv);
        ~MocCtx();

        int runMoc();
    };
}