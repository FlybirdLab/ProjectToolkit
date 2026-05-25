/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GraphicsSandbox.hpp"
#include "NetworkSandbox.hpp"

#include <toolkit/debug/Logger.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/utils/CommandLine.hpp>

using namespace toolkit;

namespace
{
    void printUsage()
    {
        LogInfo("Usage: Sandbox [graphics|net] [command] [options]");
        LogInfo("  graphics  Run the graphics triangle sample (default).");
        LogInfo("            [--backend=dx12|dx11|vulkan]  Pick which compiled-in backend to use.");
        LogInfo("            [--list-backends]             List backends compiled into this build.");
        LogInfo("  net check                                   Run NetKit local checks (default).");
        LogInfo("  net server [--port=7777] [--timeout=30]    Listen and reply to one client ping.");
        LogInfo("  net client [--address=127.0.0.1] [--port=7777]");
        LogInfo("             [--message=text] [--timeout=10] Connect, ping and verify the reply.");
    }
}

int main(int argc, char **argv)
{
    if (!System::init())
        return 1;
    System::beforeRun();

    const TkCommandLine commandLine(argc, argv);
    const TkArray<TkString> &positionals = commandLine.getPositionals();
    const TkString mode = positionals.hasItems() ? positionals[0] : TkString("help");

    int result = 0;
    if (commandLine.has("help") || commandLine.has("h"))
    {
        printUsage();
    }
    else if (mode == "graphics" || mode == "render")
    {
        GraphicsSandbox sandbox;
        result = sandbox.run(commandLine);
    }
    else if (mode == "net" || mode == "network")
    {
        NetworkSandbox sandbox;
        result = sandbox.run(commandLine);
    }
    else
    {
        LogError("Unknown Sandbox mode: %s", mode.getText());
        printUsage();
        result = 1;
    }

    System::beforeQuit();
    System::quit();
    return result;
}
