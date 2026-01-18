#include <mikmod.h>

int AudioInit(void)
{
    MikMod_RegisterDriver(&drv_sb);
    MikMod_RegisterAllLoaders();

    md_mode |= DMODE_SOFT_MUSIC;
    if (MikMod_Init(""))
    {
        fprintf(stderr, "Could not initialize sound, reason: %s\n",
        MikMod_strerror(MikMod_errno));
        return -1;
    }

    return 0;
}

void AudioShutdown(void)
{
    MikMod_Exit();
}

void AudioUpdate(void)
{
    MikMod_Update();
}