#include "testApp.h"
#include "iostream.h"

#define NROT 180



//--------------------------------------------------------------
void testApp::setup(){

    ofDirectory dir;
    
    dir.listDir("output");
    for (int i = 0; i < dir.size(); i++){
        string fileName = dir.getPath(i);
        lineSets.push_back( linesetFromFile(fileName) );
        lineSets[lineSets.size()-1].idMe = i;
        
        
        //int nRot = 100;
        ofMatrix4x4 mat;
        mat.makeRotationMatrix(360 / NROT, 0, 0, 1);
        
        ofPolyline temp = lineSets[lineSets.size()-1].normalized;
        
        
        for (int i = 0; i < NROT; i++){
            for (int j = 0; j < temp.size(); j++){
               temp[j] = temp[j] * mat;
            }
            ofPolyline * newLine = new ofPolyline();
            *newLine = temp;
            lineSets[lineSets.size()-1].normalizedDiffAngles.push_back(newLine);
        }
        
        distanceResult res;
        res.id = i;
        res.distance = 0;
        distanceResults.push_back(res);
    }
    
    
    for (int i = 0; i < lineSets.size(); i++){
        
        for (int j = 0; j < lineSets[i].normalizedDiffAngles.size(); j++){
            polyPtr pp;
            pp.distance = 0;
            pp.line = lineSets[i].normalizedDiffAngles[j];
            pp.whichAngle = j;
            pp.whichLine = i;
            
            polyPtrs.push_back(pp);
        }
    }
    
    
    CL.setup();
    
    lastMatchTime = ofGetElapsedTimef();
    
    ofSetVerticalSync(true);
    
    TIME_SAMPLE_SET_FRAMERATE( 60.0f );
    
    
    shader.load("shader/shader");
    
    angleCatch = 0;
}

//--------------------------------------------------------------
void testApp::update(){
    
    if (ofGetFrameNum() % 30 == 0)shader.load("shader/shader");
    
    TIME_SAMPLE_START("update");
    
    CL.update();
    
    
    if (ofGetMousePressed()){
        
        CL.translateNodeLine(ofPoint(-1, 0));
        
        for (int i = 0; i < matchStructs.size(); i++){
            matchStructs[i].matchA.x -= 1.0;
            matchStructs[i].matchB.x -= 1.0;
            matchStructs[i].offset.x -= 1.0;
        }
    
    } else {
        
        CL.translateNodeLine(ofPoint(-1, 0));
        
        for (int i = 0; i < matchStructs.size(); i++){
            matchStructs[i].matchA.x -= 1.0;
            matchStructs[i].matchB.x -= 1.0;
            matchStructs[i].offset.x -= 1.0;
        }

        
    }
    
    if (ofGetFrameNum() % 5 == 0 && CL.nodeLine.size() > 100){
        
        
        TIME_SAMPLE_START("match");
        lookForGoodMatch();
        TIME_SAMPLE_STOP("match");
    }
    
    TIME_SAMPLE_STOP("update");
}

//--------------------------------------------------------------
void testApp::lookForGoodMatch(){
    nodeLineLast100.clear();
    for (int i = CL.nodeLine.size()-100; i < CL.nodeLine.size(); i++){
        nodeLineLast100.addVertex(CL.nodeLine[i]);
    }
    nodeLine40 = nodeLineLast100.getResampledByCount(40);
    nodeLine40 = returnNormalizedLine(nodeLine40);
    

    for (int i = 0; i < polyPtrs.size(); i++){
        distancePP(nodeLine40, polyPtrs[i], 2);
    }
    sort(polyPtrs.begin(), polyPtrs.end(),sortFuncPP);
    
    for (int i = 0; i < polyPtrs.size()/4; i++){
        distancePP(nodeLine40, polyPtrs[i], 6);
    }
    sort(polyPtrs.begin(), polyPtrs.begin() + polyPtrs.size()/4,sortFuncPP);
    
    
    for (int i = 0; i < polyPtrs.size()/20; i++){
        distancePP(nodeLine40, polyPtrs[i], 20);
    }
    
    for (int i = 0; i < lastFound.size(); i++){
        for (int j = 0; j < polyPtrs.size()/20; j++){
            if (polyPtrs[j].whichLine == lastFound[i]){
                polyPtrs[j].distance = 100000000;
            }
        }
    }
    
    sort(polyPtrs.begin(), polyPtrs.begin() + polyPtrs.size()/20,sortFuncPP);
    
    //ofSort(polyPtrs, sortFuncPP);
    //ofSort(distanceResults, sortFunc);
    
    cout << sqrt(polyPtrs[0].distance) << endl;
    
    if (sqrt(polyPtrs[0].distance) < 10){
        
        if (ofGetElapsedTimef()- lastMatchTime > 3.0){
            matchStruct match;
            match.matchA = nodeLineLast100.getVertices()[0];
            match.matchB = nodeLineLast100.getVertices()[99];
            match.idOfMatch = polyPtrs[0].whichLine;
            ofPolyline new40 = nodeLineLast100.getResampledByCount(40);;
            match.trans = normalizeLineSetGetTrans(new40);
            match.offset.set(0,0,0);
            match.startTime = ofGetElapsedTimef();
            match.matchLine = nodeLine40;
            match.bestAngle = polyPtrs[0].whichAngle;
            matchStructs.push_back(match);
            
            if (matchStructs.size() > 8) matchStructs.erase(matchStructs.begin());
            
            lastFound.push_back(polyPtrs[0].whichLine);
            if (lastFound.size() > 10) lastFound.erase(lastFound.begin());
            
            lastMatchTime = ofGetElapsedTimef();
        }
    }
    //cout << distanceResults[0].id << endl;
    //cout << lineSets[0].idMe << " " << lineSets[1].idMe << endl;
    
}


//--------------------------------------------------------------
void testApp::draw(){

    //return;
    
    
    

    TIME_SAMPLE_START("draw");
    
    shader.begin();
    
    
    
    if (CL.nodeLine.size()> 0){
        
    ofPoint pt = CL.nodeLine[CL.nodeLine.size()-1];
    ofMatrix4x4 mat;
    ofMatrix4x4 t1;
    ofMatrix4x4 t2;
    t1.makeTranslationMatrix( -pt);
    t2.makeTranslationMatrix( ofPoint(ofGetWidth()/2, ofGetHeight()/2));
    ofMatrix4x4 res;

        float angle = 0;
        if (matchStructs.size() > 0){
            angle = (360 / NROT) * matchStructs[matchStructs.size()-1].bestAngle;
            
            //ofMatrix4x4 rotate; rotate.makeRotationMatrix( (360 / NROT) * match.bestAngle, 0, 0, 1 );
        }
        
        
        angleCatch = ofLerpDegrees(angleCatch, angle, 0.01);
        
    mat.makeRotationMatrix(-angleCatch, 0, 0, 1);
    
    res = t1 * mat * t2;
    
    shader.setUniformMatrix4f("matrix", res);
    } else {
       // ofMatrix4x4 mat;
        
       //shader.setUniformMatrix
    }
    
    ofBackground(0);
    ofPushStyle();
    //ofSetColor(ofColor::darkGrey);
    
    //ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    //ofScale( 0.2, 0.2 );
    
    
//    ofPolyline temp;
//    temp =CL.nodeLine.getResampledBySpacing(8);
//    
//    for (int i = 0; i < temp.size(); i++){
//        ofRect(temp[i].x, temp[i].y, 2,2);
//    }
    CL.nodeLine.draw();
    
//    for (int i = 0; i < CL.nodeLine.size()-1; i++){
//        
//        if (i % 5 == 0){
//         
//            //ofPoint(CL.nodeLine[i]);
//            ofPoint a(CL.nodeLine[i+1] - CL.nodeLine[i+1]);
//            ofLine(CL.nodeLine[i], CL.nodeLine[i+1]);
//        }
//    }
    
    
    
    for (int i = 0; i < matchStructs.size(); i++){
        
        //cout << matchStructs[i].offset << endl;
        //if (matchStructs[i].offset.x < -400) continue;

        drawLineSet( lineSets[matchStructs[i].idOfMatch],matchStructs[i], matchStructs[i].matchA, matchStructs[i].matchB);

        ofNoFill();
        //ofRect(matchStructs[i].bounds);
        
        
        ofSetColor(ofColor::pink);
        ofPushMatrix();
        
        ofTranslate(mouseX, mouseY);
        
        if (i == matchStructs.size()-1){
            //matchStructs[i].matchLine.draw();
        }
        ofPopMatrix();
        //ofLine(matchStructs[i].matchA, matchStructs[i].matchB);
    }
    
    ofPushMatrix();
    

    
    ofPopMatrix();
    
    ofPopStyle();
    
    shader.end();
    
    TIME_SAMPLE_STOP("draw"); /////////////////////////////////////////////////  STOP MEASURING  ///
	
	TIME_SAMPLE_DRAW( 10, 10); 	//finally draw our time measurements


}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}


void testApp::drawLineSet(lineSet & set, matchStruct & match, ofPoint ptA, ofPoint ptB){
    
    
    //int nRot = 100;
    
    
    ofMatrix4x4 inverse = match.trans.mat.getInverse();
    ofMatrix4x4 inverse2 = match.trans.mat2.getInverse();
    
    //cout << match.bestAngle << endl;
    ofMatrix4x4 rotate; rotate.makeRotationMatrix( (360 / NROT) * match.bestAngle, 0, 0, 1 );
    
    float pct = ofMap( ofGetElapsedTimef()- match.startTime, 0, 1.5, 0, 1, true);
    
    int nTotal = 0;
    for (int i = 0; i < set.normalizeLines.size(); i++){
        nTotal +=set.normalizeLines[i].size();
    }
    
    int nCountTo = nTotal * pct;
    
    int count = 0;
    
    //match.bounds.set( match.offset.x, match.offset.y, 1,1);
    
    
    for (int i = 0; i < set.normalizeLines.size(); i++){
        
        ofEnableAlphaBlending() ;
        
        if (i == 0)  ofGetMousePressed() == false ? ofSetColor(255,255,255,0) : ofSetColor(ofColor::cyan);
        else ofSetColor(ofColor::white);
        
        
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
        for (int j = 0; j < set.normalizeLines[i].size(); j++){
            
            ofPoint newPt = set.normalizeLines[i][j] * rotate * inverse2 * inverse;
            
            if (count < nCountTo){
                mesh.addVertex(newPt + match.offset);
                
                if (i > 0){
                    if (i == 1 && j == 0) {
                        match.bounds.set((newPt + match.offset).x, (newPt + match.offset).y, 1,1);
                    } else {
                        match.bounds.growToInclude(newPt + match.offset);
                        
                    }
                }
            }
            count++;
        }
        
        
        ofRectangle rect(0,0,ofGetWidth(), ofGetHeight());
        if (rect.intersects( match.bounds)){
            //cout << match.bounds << endl;
            mesh.draw();
        }
    }
}

//--------------------------------------------------------------
lineSet & testApp::linesetFromFile( string fileName ){
    
    set.lines.clear();
    
    ifstream myFile (ofToDataPath(fileName).c_str(), ios::out | ios::binary);
    
    int howMany;
    myFile.read((char *)&howMany, sizeof(int));
    
    int * lengths = new int[howMany];
    for(int i = 0; i < howMany; i++){
        myFile.read((char *)&lengths[i], sizeof(int));
    }
    
    for(int i = 0; i < howMany; i++){
        ofPolyline line;
        float pt[2];
        for(int j = 0; j < lengths[i]; j++){
            
            myFile.read((char *)&pt[0], sizeof(float));
            myFile.read((char *)&pt[1], sizeof(float));
            
            line.addVertex(ofPoint(pt[0], pt[1]));
        }
        
        set.lines.push_back(line);
    }
    
    myFile.close();
   
    ofPolyline lineAt40 = set.lines[0].getResampledByCount(40);
    set.normalized = returnNormalizedLine(lineAt40);
    normalizeLineSet(set, lineAt40);
    
    return set;
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
    
    
    
    

}