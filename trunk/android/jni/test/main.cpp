//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <ting/Array.hpp>
#include <ting/Buffer.hpp>
#include <ting/debug.hpp>
#include <ting/Exc.hpp>
#include <ting/File.hpp>
#include <ting/FSFile.hpp>
#include <ting/math.hpp>
#include <ting/PoolStored.hpp>
#include <ting/Ptr.hpp>
#include <ting/Ref.hpp>
#include <ting/Signal.hpp>
#include <ting/Singleton.hpp>
#include <ting/Socket.hpp>
#include <ting/Thread.hpp>
#include <ting/Timer.hpp>
#include <ting/types.hpp>
#include <ting/utils.hpp>
#include <ting/WaitSet.hpp>



#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))




















namespace TestBoolRelatedStuff{

class TestClass : public ting::RefCounted{
public:
	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}
};



static void TestConversionToBool(){
	ting::Ref<TestClass> a;
	ting::Ref<TestClass> b = TestClass::New();

	//test conversion to bool
	if(a){
		ASSERT_ALWAYS(false)
	}

	if(b){
	}else{
		ASSERT_ALWAYS(false)
	}
}



static void TestOperatorLogicalNot(){
	ting::Ref<TestClass> a;
	ting::Ref<TestClass> b = TestClass::New();

	//test operator !()
	if(!a){
	}else{
		ASSERT_ALWAYS(false)
	}

	if(!b){
		ASSERT_ALWAYS(false)
	}
}

}//~namespace



namespace TestOperatorArrowAndOperatorStar{
class A : public ting::RefCounted{
public:
	int a;

	A() : a(98)
	{}

	static inline ting::Ref<A> New(){
		return ting::Ref<A>(new A());
	}
};


static void Run1(){
	ting::Ref<A> a = A::New();
	ting::Ref<const A> ac = A::New();

	ASSERT_ALWAYS(a)
	ASSERT_ALWAYS(ac)

	ASSERT_ALWAYS(ac != a)
	ASSERT_ALWAYS(a != ac)

	a->a = 123;
	ASSERT_ALWAYS((*a).a == 123)

	(*a).a = 456;
	ASSERT_ALWAYS(a->a == 456)

	ASSERT_ALWAYS(ac->a == 98)
	ASSERT_ALWAYS((*ac).a == 98)

	ac = a;

	ASSERT_ALWAYS(ac == a)
	ASSERT_ALWAYS(a == ac)
}

static void Run2(){
	const ting::Ref<A> a = A::New();
	const ting::Ref<const A> ac = A::New();

	ASSERT_ALWAYS(a)
	ASSERT_ALWAYS(ac)

	ASSERT_ALWAYS(ac != a)
	ASSERT_ALWAYS(a != ac)

	a->a = 123;
	ASSERT_ALWAYS((*a).a == 123)

	(*a).a = 456;
	ASSERT_ALWAYS(a->a == 456)

	ASSERT_ALWAYS(ac->a == 98)
	ASSERT_ALWAYS((*ac).a == 98)
}

}//~namespace



namespace TestBasicWeakRefUseCase{
class TestClass : public ting::RefCounted{
public:
	bool *destroyed;

	TestClass() :
		destroyed(0)
	{}

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}

	~TestClass(){
		if(this->destroyed){
			*this->destroyed = true;
		}
	}
};

static void Run1(){
	for(unsigned i = 0; i < 1000; ++i){
		ting::Ref<TestClass> a = TestClass::New();
		ASSERT_ALWAYS(a.IsValid())

		bool wasDestroyed = false;
		a->destroyed = &wasDestroyed;

		ting::WeakRef<TestClass> wr(a);
		ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsValid())
		ASSERT_ALWAYS(!wasDestroyed)

		a.Reset();

		ASSERT_ALWAYS(a.IsNotValid())
		ASSERT_ALWAYS(wasDestroyed)
		ASSERT_INFO_ALWAYS(ting::Ref<TestClass>(wr).IsNotValid(), "i = " << i)
	}//~for
}

static void Run2(){
	ting::Ref<TestClass> a = TestClass::New();
	ASSERT_ALWAYS(a.IsValid())

	bool wasDestroyed = false;
	a->destroyed = &wasDestroyed;

	ting::WeakRef<TestClass> wr1(a);
	ting::WeakRef<TestClass> wr2(wr1);
	ting::WeakRef<TestClass> wr3;

	wr3 = wr1;

	ASSERT_ALWAYS(ting::Ref<TestClass>(wr1).IsValid())
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr2).IsValid())
	ASSERT_ALWAYS(wr3.GetRef().IsValid())

	a.Reset();

	ASSERT_ALWAYS(a.IsNotValid())
	ASSERT_ALWAYS(wasDestroyed)

	ASSERT_ALWAYS(ting::Ref<TestClass>(wr1).IsNotValid())
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr2).IsNotValid())
	ASSERT_ALWAYS(wr3.GetRef().IsNotValid())
}

}//~namespace



namespace TestExceptionThrowingFromRefCountedDerivedClassConstructor{

class TestClass : public ting::RefCounted{
public:
	TestClass(){
		throw ting::Exc("TestException!");
	}

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}
};



static void Run(){
	try{
		ting::Ref<TestClass> a = TestClass::New();
		ASSERT_ALWAYS(false)
	}catch(ting::Exc&){
		//do nothing
	}
}

}//~namespace



namespace TestCreatingWeakRefFromRefCounted{

class TestClass : public ting::RefCounted{
public:
	bool *destroyed;

	TestClass() :
			destroyed(0)
	{
	}

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}

	static inline TestClass* SimpleNew(){
		return new TestClass();
	}

	~TestClass(){
		if(this->destroyed)
			*this->destroyed = true;
	}
};



void Run1(){
//	TRACE(<< "TestCreatingWeakRefFromRefCounted::Run(): enter" << std::endl)
	
	bool destroyed = false;
	
	TestClass *tc = TestClass::SimpleNew();
	ASSERT_ALWAYS(tc)
	tc->destroyed = &destroyed;

	ting::WeakRef<TestClass> wr(tc);
	ASSERT_ALWAYS(!destroyed)

	ting::WeakRef<ting::RefCounted> wrrc(tc);
	ASSERT_ALWAYS(!destroyed)

	wr.Reset();
	ASSERT_ALWAYS(!destroyed)

	wrrc.Reset();
	ASSERT_ALWAYS(!destroyed)

	wr = tc;//operator=()
	ASSERT_ALWAYS(!destroyed)

	//there is 1 weak reference at this point

	ting::Ref<TestClass> sr(tc);
	ASSERT_ALWAYS(!destroyed)

	sr.Reset();
	ASSERT_ALWAYS(destroyed)
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsNotValid())
}

void Run2(){
	bool destroyed = false;

	TestClass *tc = TestClass::SimpleNew();
	ASSERT_ALWAYS(tc)
	tc->destroyed = &destroyed;

	ting::WeakRef<TestClass> wr(tc);
	ASSERT_ALWAYS(!destroyed)

	wr.Reset();
	ASSERT_ALWAYS(!destroyed)

	//no weak references at this point

	ting::Ref<TestClass> sr(tc);
	ASSERT_ALWAYS(!destroyed)

	sr.Reset();
	ASSERT_ALWAYS(destroyed)
	ASSERT_ALWAYS(ting::Ref<TestClass>(wr).IsNotValid())
}

}//~namespace



namespace TestAutomaticDowncasting{
class A : public ting::RefCounted{
public:
	int a;

	A() : a(0)
	{}
};

class B : public A{
public:
	int b;

	B() : b(0)
	{}
};

class C : public B{
public:
	int c;

	C() : c(0)
	{}

	static ting::Ref<C> New(){
		return ting::Ref<C>(new C());
	}
};



void Run1(){
	ting::Ref<C> c = C::New();
	ASSERT_ALWAYS(c)

	const int DConstA = 1;
	const int DConstB = 3;
	const int DConstC = 5;

	c->a = DConstA;
	c->b = DConstB;
	c->c = DConstC;

	//A
	{
		ting::Ref<A> a(c);
		ASSERT_ALWAYS(a.IsValid())
		ASSERT_ALWAYS(a->a == DConstA)

		ASSERT_ALWAYS(a.DynamicCast<B>())
		ASSERT_ALWAYS(a.DynamicCast<B>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<B>()->b == DConstB)

		ASSERT_ALWAYS(a.DynamicCast<C>())
		ASSERT_ALWAYS(a.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(a.DynamicCast<C>()->c == DConstC)
	}

	{
		ting::Ref<A> a;
		a = c;
		ASSERT_ALWAYS(a.IsValid())
		ASSERT_ALWAYS(a->a == DConstA)

		ASSERT_ALWAYS(a.DynamicCast<B>())
		ASSERT_ALWAYS(a.DynamicCast<B>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<B>()->b == DConstB)

		ASSERT_ALWAYS(a.DynamicCast<C>())
		ASSERT_ALWAYS(a.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(a.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(a.DynamicCast<C>()->c == DConstC)
	}

	//B
	{
		ting::Ref<B> b(c);
		ASSERT_ALWAYS(b.IsValid())
		ASSERT_ALWAYS(b->a == DConstA)
		ASSERT_ALWAYS(b->b == DConstB)

		ASSERT_ALWAYS(b.DynamicCast<C>())
		ASSERT_ALWAYS(b.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(b.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(b.DynamicCast<C>()->c == DConstC)
	}

	{
		ting::Ref<B> b;
		b = c;
		ASSERT_ALWAYS(b.IsValid())
		ASSERT_ALWAYS(b->a == DConstA)
		ASSERT_ALWAYS(b->b == DConstB)

		ASSERT_ALWAYS(b.DynamicCast<C>())
		ASSERT_ALWAYS(b.DynamicCast<C>()->a == DConstA)
		ASSERT_ALWAYS(b.DynamicCast<C>()->b == DConstB)
		ASSERT_ALWAYS(b.DynamicCast<C>()->c == DConstC)
	}
}

}//~namespace


namespace TestVirtualInheritedRefCounted{
class A : virtual public ting::RefCounted{
public:
	int a;
};

class B : virtual public ting::RefCounted{
public:
	int b;
};

class C : public A, B{
public:
	int c;

	bool& destroyed;

	C(bool& destroyed) :
			destroyed(destroyed)
	{}

	~C(){
		this->destroyed = true;
	}

	static ting::Ref<C> New(bool& destroyed){
		return ting::Ref<C>(new C(destroyed));
	}
};

void Run1(){
	bool isDestroyed = false;

	ting::Ref<C> p = C::New(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}

void Run2(){
	bool isDestroyed = false;

	ting::Ref<A> p = C::New(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}

void Run3(){
	bool isDestroyed = false;

	ting::Ref<ting::RefCounted> p = C::New(isDestroyed);

	ASSERT_ALWAYS(!isDestroyed)
	ASSERT_ALWAYS(p.IsValid())

	p.Reset();

	ASSERT_ALWAYS(p.IsNotValid())
	ASSERT_ALWAYS(isDestroyed)
}
}//~namespace



namespace TestConstantReferences{
class TestClass : public ting::RefCounted{
public:
	int a;

	mutable int b;

	static inline ting::Ref<TestClass> New(){
		return ting::Ref<TestClass>(new TestClass());
	}
};

void Run1(){
	ting::Ref<TestClass> a = TestClass::New();
	ting::Ref<const TestClass> b(a);

	ASSERT_ALWAYS(a)
	ASSERT_ALWAYS(b)

	a->a = 1234;
	a->b = 425345;
	
	b->b = 2113245;

	{
		ting::WeakRef<TestClass> wa(a);
		ting::WeakRef<const TestClass> wb(a);
		ting::WeakRef<const TestClass> wb1(b);

		ASSERT_ALWAYS(ting::Ref<TestClass>(wa)->a == 1234)
		ASSERT_ALWAYS(ting::Ref<const TestClass>(wb)->a == 1234)
		ASSERT_ALWAYS(ting::Ref<const TestClass>(wb1)->a == 1234)
		ASSERT_ALWAYS(wb1.GetRef()->a == 1234)
	}

	{
		ting::WeakRef<TestClass> wa(a);
		ting::WeakRef<const TestClass> wb(wa);

		ASSERT_ALWAYS(ting::Ref<const TestClass>(wb)->a == 1234)
	}
}
}//~namespace






















namespace TestFetchAndAdd{

const unsigned DNumOps = 0xffff;

class Thread : public ting::Thread{
	ting::atomic::S32 &a;
public:
	Thread(ting::atomic::S32 &a) :
			a(a)
	{}
	
	ting::Semaphore sema;
	
	//override
	void Run(){
//		TRACE(<< "Thread::Run(): enter" << std::endl)
		
		//wait for start signal
		this->sema.Wait();
		
		for(unsigned i = 0; i < DNumOps; ++i){
			this->a.FetchAndAdd(1);
		}
	}
};


void Run(){
	ting::atomic::S32 a;
	
	ting::StaticBuffer<ting::Ptr<Thread>, 100> threads;
	
	//Create and start all the threads
	for(ting::Ptr<Thread>* i = threads.Begin(); i != threads.End(); ++i){
		(*i) = ting::Ptr<Thread>(new Thread(a));
		(*i)->Start();
	}
	
	//wait till all the threads enter their Run() methods and start waiting on the semaphores
	ting::Thread::Sleep(500);
	
	//signal all threads semaphores
//	TRACE(<< "Signalling..." << std::endl)
	for(ting::Ptr<Thread>* i = threads.Begin(); i != threads.End(); ++i){
		(*i)->sema.Signal();
	}
	
	//wait for all threads to finish
	for(ting::Ptr<Thread>* i = threads.Begin(); i != threads.End(); ++i){
		(*i)->Join();
	}
//	TRACE(<< "All threads finished" << std::endl)
	
	//Check atomic value
	ASSERT_ALWAYS(a.FetchAndAdd(0) == ting::s32(DNumOps * threads.Size()))
}
}//~namespace



namespace TestCompareAndExchange{
void Run(){
	ting::atomic::S32 a(10);
	
	ASSERT_ALWAYS(a.CompareAndExchange(10, 9) == 10)
	ASSERT_ALWAYS(a.CompareAndExchange(9, 10) == 9)
	
	
}
}//~namespace



namespace TestFlag{
void Run(){
	ting::atomic::Flag f;
	
	ASSERT_ALWAYS(f.Get() == false)
	
	ASSERT_ALWAYS(f.Set(false) == false)
	ASSERT_ALWAYS(f.Set(true) == false)
	ASSERT_ALWAYS(f.Set(true) == true)
	ASSERT_ALWAYS(f.Set(false) == true)
}
}//~namespace

























/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
            ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();
	
	
	
	
	
	TRACE_ALWAYS(<< "STARTING!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl)

	ting::TimerLib timerLib;
	
	ting::Mutex testMutex;

	ting::WaitSet testWaitset(3);

	ting::FSFile testFSFile("testfile.txt");


	ting::TCPSocket testSocket;
	
	try{
		testSocket.Open(ting::IPAddress("127.0.0.1", 80));
	}catch(std::exception& e){
		TRACE_ALWAYS(<< "exception caught: " << e.what() << std::endl)
	}
	
	
	
	TestFlag::Run();
	TestCompareAndExchange::Run();
	TestFetchAndAdd::Run();
	
	
	
	
	
	
	TestOperatorArrowAndOperatorStar::Run1();
	TestOperatorArrowAndOperatorStar::Run2();

	TestAutomaticDowncasting::Run1();

	TestBoolRelatedStuff::TestConversionToBool();
	TestBoolRelatedStuff::TestOperatorLogicalNot();
	
	TestBasicWeakRefUseCase::Run1();
	TestBasicWeakRefUseCase::Run2();

	TestExceptionThrowingFromRefCountedDerivedClassConstructor::Run();
	
	TestCreatingWeakRefFromRefCounted::Run1();
	TestCreatingWeakRefFromRefCounted::Run2();
	
	TestVirtualInheritedRefCounted::Run1();
	TestVirtualInheritedRefCounted::Run2();
	TestVirtualInheritedRefCounted::Run3();

	TestConstantReferences::Run1();
	
	
	
	
	
	

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                            &event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                                event.acceleration.x, event.acceleration.y,
                                event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
//END_INCLUDE(all)
