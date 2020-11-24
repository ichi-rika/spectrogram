#ifndef TARGET_SDL2_H
#define TARGET_SDL2_H

#include "../base/base.h"
#include <SDL2/SDL.h>

#include <mutex>

namespace Module::Target {

    class SDL2 : public AbstractBase {
    public:
        SDL2(Type rendererType);
        virtual ~SDL2();

        void initialize() override;
        void terminate() override;

        void setTitle(const std::string& title) override;
        void setSize(int width, int height) override;

        void getSize(int *pWidth, int *pHeight) override;
        void getSizeForRenderer(int *pWidth, int *pHeight) override;
        void getDisplayDPI(float *hdpi, float *vdpi, float *ddpi) override;

        void create() override;
        void show() override;
        void hide() override;
        void close() override;

        bool isVisible() const override;

        void processEvents() override;
        bool shouldQuit() override;
        bool shouldClose() override;
        bool sizeChanged() override;
        bool isKeyPressed(uint32_t key) override;
        bool isKeyPressedOnce(uint32_t key) override;
        bool isMousePressed(uint32_t key) override;
        bool isMousePressedOnce(uint32_t key) override;
        std::pair<int, int> getMousePosition() override;

        std::pair<float, float> getSwipeMovement() override;
   
        bool isTouchPressed() override;
            
    public:
#ifdef __ANDROID__
        static void prepareAssets(); 
#endif

        static std::vector<SDL_Event> globalEvents;

        Type mRendererType;

        std::string mTitle;
        int mWidth;
        int mHeight;

        SDL_Window *mWindow;
        bool mIsShown;

        bool mGotQuitEvent;
        bool mGotCloseEvent;
        bool mWindowSizeChanged;
    
        std::map<SDL_Scancode, bool> mKeyState;
        std::map<SDL_Scancode, char> mKeyupState;
    
        bool mMouseFocus;
        int mMouseX, mMouseY;
        uint32_t mMouseBitmask;
        std::map<uint32_t, char> mMouseupState;

        float mSwipeX, mSwipeY;
        bool mTouchPressed;
        uint32_t mTouchTime;
        float mTouchAbsdx;
        float mTouchAbsdy;

        int err;
        void checkError(bool cond);
    };

#ifdef RENDERER_USE_NVG
    class SDL2_NanoVG : public NvgProvider {
    public:
        SDL2_NanoVG(SDL_Window **ptrWindow);

        NVGcontext *createContext(int flags) override;
        void deleteContext(NVGcontext *ctx) override;
       
        void beforeBeginFrame() override; 
        void afterEndFrame() override;
    
        void *createFramebuffer(NVGcontext *ctx, int width, int height, int imageFlags) override;
        void bindFramebuffer(void *framebuffer) override;
        void deleteFramebuffer(void *framebuffer) override;
        int framebufferImage(void *framebuffer) override;

    private:
        SDL_Window **mPtrWindow;
        
        void createRenderer();
        void destroyRenderer();
        SDL_Renderer *mRenderer;

        void createGLContext();
        void destroyGLContext();
        SDL_GLContext mGlContext;
    };
#endif

}

#endif // VIDEOTARGET_SDL2_H
