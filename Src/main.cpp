#include "mainwindow.h"
#include <QApplication>

#include <thread>

int SourceWidth=960;
int SourceHeight=720;

static void VX_CALLBACK myLogCallback(vx_context , vx_reference , vx_status , const vx_char string[])
{
    std::cout << "VisionWorks LOG : " << string << std::endl;
}

void GenerateGraph(vx_context context, vx_graph &graph, vx_image &src1, vx_image &output)
{
    graph = vxCreateGraph(context);
    NVXIO_CHECK_REFERENCE(graph);

    vx_uint32 width=SourceWidth;
    vx_uint32 height=SourceHeight;


    vx_image grayImage = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_U8);
    NVXIO_CHECK_REFERENCE(grayImage);

    vx_node nodeYuv = vxColorConvertNode(graph,src1,grayImage);
    NVXIO_CHECK_REFERENCE(nodeYuv);

    vx_threshold cannyThreshold = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_INT32);
    NVXIO_CHECK_REFERENCE(cannyThreshold);

    vx_node nodeCanny = vxCannyEdgeDetectorNode(graph, grayImage,cannyThreshold,3, VX_NORM_L1, output );
    NVXIO_CHECK_REFERENCE(nodeCanny);

    //
    // Ensure highest graph optimization level
    //

    const char* option = "-O3";
    NVXIO_SAFE_CALL( vxSetGraphAttribute(graph, NVX_GRAPH_VERIFY_OPTIONS, option, strlen(option)) );

    //
    // Verify the graph
    //

    vx_status status = vxVerifyGraph(graph);

    if (status != VX_SUCCESS)
    {
        std::cerr << "Error: Graph verification failed (see LOG)" << std::endl;
    }
}

struct EventData
{
    EventData(): shouldStop(false), pause(false) {}

    bool shouldStop;
    bool pause;
};

static void eventCallback(void* eventData, vx_char key, vx_uint32, vx_uint32)
{
    EventData* data = static_cast<EventData*>(eventData);

    if (key == 27)
    {
        data->shouldStop = true;
    }
    else if (key == 32)
    {
        data->pause = !data->pause;
    }
    else if(key==96){
       // confi
    }
}

void opencvLog(const char string[])
{
    std::cout<< "OpencvLog: " << string<< '\n';
}



static void displayState(nvxio::Render *renderer,
                         const nvxio::FrameSource::Parameters &sourceParams,
                         double proc_ms, double total_ms,double fetchTime)
{
    std::ostringstream txt;

    txt << std::fixed << std::setprecision(1);

    nvxio::Render::TextBoxStyle style = {{255, 50, 50, 200}, {0, 0, 0, 127}, {10, 10}};

    txt << "Source size: " << sourceParams.frameWidth << 'x' << sourceParams.frameHeight << std::endl;
    txt << "Fetch: " << fetchTime << " ms / " << 1000.0 / fetchTime << " FPS" << std::endl;
    txt << "Algorithm: " << proc_ms << " ms / " << 1000.0 / proc_ms << " FPS" << std::endl;
    txt << "Total: " << total_ms  << " ms / " << 1000.0 / total_ms << " FPS" << std::endl;

    txt << std::setprecision(6);
    txt.unsetf(std::ios_base::floatfield);

    txt << "Space - pause/resume" << std::endl;
    txt << "Esc - close the demo";
    renderer->putTextViewport(txt.str(), style);
}

int VxThread(EventData eventData)
{
    int argc;
    char* argv[10];
    nvxio::Application &app = nvxio::Application::get();
    nvxio::ContextGuard context;
    vxRegisterLogCallback(context, &myLogCallback, vx_false_e);

    //
    // Parse command line arguments.The input video filename is read into
    // sourceURI and the configuration parameters are read into configFile
    //

    std::string sourceUri = "device:///v4l2?index=0";
    std::string configFile = "feature_tracker_demo_config.ini";
    app.setDescription("This demo demonstrates Feature Tracker algorithm");
    app.addOption('s', "source", "Source URI", nvxio::OptionHandler::string(&sourceUri));
    app.addOption('c', "config", "Config file path", nvxio::OptionHandler::string(&configFile));
    app.init(argc, argv);

    //
    // Create a NVXIO-based frame source
    //

    std::unique_ptr<nvxio::FrameSource> source(
        nvxio::createDefaultFrameSource(context, sourceUri));

    nvxio::FrameSource::Parameters sourceParams = source->getConfiguration();
    sourceParams.frameWidth=SourceWidth;
    sourceParams.frameHeight=SourceHeight;
    source->setConfiguration(sourceParams);

    if (!source || !source->open())
    {
        std::cerr << "Error: Can't open source URI " << sourceUri << std::endl;
        return nvxio::Application::APP_EXIT_CODE_NO_RESOURCE;
    }

    if (source->getSourceType() == nvxio::FrameSource::SINGLE_IMAGE_SOURCE)
    {
        std::cerr << "Error: Can't work on a single image." << std::endl;
        return nvxio::Application::APP_EXIT_CODE_INVALID_FORMAT;
    }

    //Creating openVx images
    vx_image imageInput = vxCreateImage(context, sourceParams.frameWidth, sourceParams.frameHeight, VX_DF_IMAGE_RGBX);
    NVXIO_CHECK_REFERENCE(imageInput);

    source->fetch(imageInput);
    vx_image imageOutput = vxCreateImage(context,sourceParams.frameWidth, sourceParams.frameHeight, VX_DF_IMAGE_U8);
    NVXIO_CHECK_REFERENCE(imageOutput);

    nvx::Timer timerFetchFrame, timerProcess, timerTotal;

    vx_graph graph;
    GenerateGraph(context, graph, imageInput, imageOutput);

    std::unique_ptr<nvxio::Render> renderer(nvxio::createDefaultRender(
        context, "", sourceParams.frameWidth, sourceParams.frameHeight));

    if (!renderer)
    {
        std::cerr << "Error: Can't create a renderer" << std::endl;
        return nvxio::Application::APP_EXIT_CODE_NO_RENDER;
    }

    renderer->setOnKeyboardEventCallback(eventCallback, &eventData);
    timerTotal.tic();

    double fetchTime,processTime,totalTime;
    nvxio::FrameSource::FrameStatus frameStatus;

    while(!eventData.shouldStop)
    {
        if(!eventData.pause)
        {
            try
            {
                timerFetchFrame.tic();
                //Fetching new frame
                frameStatus = source->fetch(imageInput);
                fetchTime=timerFetchFrame.toc();

                if (frameStatus == nvxio::FrameSource::TIMEOUT) {
                    continue;
                }
                if (frameStatus == nvxio::FrameSource::CLOSED) {
                    if (!source->open()) {
                        std::cerr << "Error: Failed to reopen the source" << std::endl;
                        break;
                    }
                    continue;
                }
                timerProcess.tic();
                //processing graph
                vxProcessGraph(graph);
                processTime=timerProcess.toc();

                //Putting data on renderer
                renderer->putImage(imageOutput);


                totalTime=timerTotal.toc();
                timerTotal.tic();
                //Displaying State
                displayState(renderer.get(), sourceParams, processTime, totalTime,fetchTime);

                //Flushing data on renderer
                renderer->flush();

            }
            catch(std::exception ex)
            {
                std::cout << ex.what();
                return nvxio::Application::APP_EXIT_CODE_ERROR;
            }
        }
     }
  //  vxReleaseImage(imageInput);
  //  vxReleaseImage(imageOutput);
  //  vxReleaseGraph(graph);
  //  vxReleaseContext(context);
    qApp->quit();
    return nvxio::Application::APP_EXIT_CODE_SUCCESS;
}




int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();    
    EventData eventData;
    std::thread VisionWorksThread(VxThread, eventData);
    a.exec();
    VisionWorksThread.join();
    return 0;
}

