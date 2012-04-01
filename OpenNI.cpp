#include "OpenNI.h"




namespace cinder {
	
	OpenNI::OpenNI(): mObj( new Obj() ){
	}

	void OpenNI::start(){
		mObj->start();
	}
	
	void OpenNI::addGesture( std::string gesture ){
		mObj->gestureGenerator.AddGesture( gesture.c_str(), NULL ); 
	}
	void OpenNI::removeGesture( std::string gesture ){
		mObj->gestureGenerator.RemoveGesture( gesture.c_str() ); 
	}
	vector<std::string> OpenNI::listGestures(){
        XnUInt16 nGestures; 
        XnChar **gestures = new XnChar*[14]; 

		for(int i=0; i<14; i++) gestures[i] = "Ceci n est pas une gesture"; 

		//mObj->gestureGenerator.EnumerateGestures
		xnEnumerateGestures( mObj->gestureGenerator,  gestures, &nGestures );

		printf( "Et voila %i gestures\n", nGestures );
		vector<std::string> output;
		for(int i=0; i<nGestures; i++) {
			cout<<gestures[i];
		}

		return output;
	}
	
	void OpenNI::startHandsTracking(Vec3f position){
		XnPoint3D pos;
		pos.X = position.x;
		pos.Y = position.y;
		pos.Z = position.z;

		mObj->handsGenerator.StartTracking( pos ); 
	}
	
	void OpenNI::stopHandsTracking(int userId){
		mObj->handsGenerator.StopTracking(userId);
	}
	void OpenNI::stopAllHandsTracking(){
		mObj->handsGenerator.StopTrackingAll();
	}
	OpenNI::Obj::Obj(){
		status = context.Init(); 
		checkStatus();
		mThreadRunning = true;
	}
	OpenNI::Obj::~Obj(){
		context.Shutdown();
		mThreadRunning = false;
		mThread->join();
	}
	
	void OpenNI::Obj::addHandsNode(){
		status = handsGenerator.Create(context); 
		checkStatus();
		status = context.FindExistingNode(XN_NODE_TYPE_HANDS, handsGenerator);
		checkStatus();

		handsGenerator.RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, this, handsCallbackHandle); 
	}

	void OpenNI::Obj::addGestureNode(){
		status = gestureGenerator.Create(context); 
		checkStatus();
		status = context.FindExistingNode(XN_NODE_TYPE_GESTURE, gestureGenerator);
		checkStatus();
		
		gestureGenerator.RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, this, gesturesCallbackHandle); 
	}

	

	void OpenNI::Obj::start(){
		status = context.StartGeneratingAll();
		checkStatus();
		   
		mThread = std::shared_ptr<boost::thread>(new boost::thread(&OpenNI::Obj::threadedFunc, this));
	}

	void OpenNI::Obj::threadedFunc(){
		while(mThreadRunning){
			boost::mutex::scoped_lock lock(mMutex); 
			status = context.WaitAndUpdateAll();
			checkStatus();
		 }
	}

	void XN_CALLBACK_TYPE OpenNI::Gesture_Recognized(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie){
		OpenNI::Obj *mObj = reinterpret_cast<OpenNI::Obj*>( pCookie );
		mObj->gestureRecognizedCallbacks.call( GestureEvent( strGesture, pEndPosition->X, pEndPosition->Y, pEndPosition->Z ) ); 
	}
	void XN_CALLBACK_TYPE OpenNI::Gesture_Process(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, XnFloat fProgress, void* pCookie){ 
		OpenNI::Obj *mObj = reinterpret_cast<OpenNI::Obj*>( pCookie );
		mObj->gestureProcessedCallbacks.call( GestureEvent( strGesture, pPosition->X, pPosition->Y, pPosition->Z ) ); 
	}

	
	void XN_CALLBACK_TYPE OpenNI::Hand_Create(xn::HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie){ 
		OpenNI::Obj *mObj = reinterpret_cast<OpenNI::Obj*>( pCookie );
        mObj->handBeganCallbacks.call(HandEvent(nId,  pPosition->X, pPosition->Y, pPosition->Z)); 
	} 
	void XN_CALLBACK_TYPE OpenNI::Hand_Update(xn::HandsGenerator& generator, XnUserID nId, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie) { 
		OpenNI::Obj *mObj = reinterpret_cast<OpenNI::Obj*>( pCookie );
        mObj->handMovedCallbacks.call(HandEvent(nId,  pPosition->X, pPosition->Y, pPosition->Z)); 
	} 

	void XN_CALLBACK_TYPE OpenNI::Hand_Destroy(xn::HandsGenerator& generator, XnUserID nId, XnFloat fTime, void* pCookie){ 
		OpenNI::Obj *mObj = reinterpret_cast<OpenNI::Obj*>( pCookie );
        mObj->handEndedCallbacks.call(HandEvent(nId,  0, 0, 0)); 
		mObj->gestureGenerator.AddGesture("Wave", NULL); 
	} 
	
	void OpenNI::Obj::checkStatus(){
		if(status != XN_STATUS_OK){
			printf("OpenNI Error: %s\n", xnGetStatusString(status)); 
		}
	}
};