#include "projector.h"

void Projector::init(int i){
    
    // defaults
    index = i;

    keyboard = false;
    mouse = false;
    xmlPrefix = "projectors/projector[";
    
    width = 1024;
    height = 768;
    
    azimuth = 0;
    elevation = 0;
    distance = 5;

    roll = 0;
    tilt = -16;
    pan = 0;
   
    fov = 33;
    
    lensOffsetX = 0;
    lensOffsetY = 0;
    
    topLeft.set(0, 0);
    topRight.set(1, 0);
    bottomLeft.set(0, 1);
    bottomRight.set(1, 1);

    scale = 1;
    
    brightness = 1;
    contrast = 1;
    saturation = 1;
    
}


void Projector::setup() {
    
    // convert camera position -> spherical to cartesian
    ofVec3f sph, car;
    sph.set(azimuth, elevation, distance);
    car = sphToCar(sph);
    
    // create camera
    camera.resetTransform();

    camera.setScale(1,-1,1); // legacy oF oddity
    camera.setFov(fov);
    camera.setPosition(car);
    camera.setNearClip(5);
        
    camera.tilt(tilt);
    camera.pan(azimuth + pan);
    camera.roll(roll);
    
    camera.setLensOffset(ofVec2f(lensOffsetX,lensOffsetY));
    
    // create view
    view.setWidth(width);
    view.setHeight(height);
    
    // create fbo
    if (fbo.getWidth() != width || fbo.getHeight() != height) {
        fbo.allocate(width, height, GL_RGB);
    }
    
    fbo.begin();
    ofClear(255);
    fbo.end();
    fboTexture = fbo.getTextureReference();
    
    // create render plane
    plane.set(width, height);
    plane.setPosition(width * index + width/2, height/2, 0);
    plane.setResolution(10, 10);
        
    int w = width;
    int h = height;
    int x = width * index;
    int y = 0;

    ofPoint tl(topLeft.x * width + x, topLeft.y * height + y);
    ofPoint tr(topRight.x * width + x, topRight.y * height + y);
    ofPoint bl(bottomLeft.x * width + x, bottomLeft.y * height + y);
    ofPoint br(bottomRight.x * width + x, bottomRight.y * height + y);
    
    keystone.setAnchorSize(width/2, height/2);
    keystone.setSourceRect( ofRectangle( 0, 0, width, height ) );              // this is the source rectangle which is the size of the image and located at ( 0, 0 )
    keystone.setTopLeftCornerPosition( tl );             // this is position of the quad warp corners, centering the image on the screen.
    keystone.setTopRightCornerPosition( tr );        // this is position of the quad warp corners, centering the image on the screen.
    keystone.setBottomLeftCornerPosition( bl );      // this is position of the quad warp corners, centering the image on the screen.
    keystone.setBottomRightCornerPosition( br ); // this is position of the quad warp corners, centering the image on the screen.
    keystone.setup();

    grid.indx = index;
    grid.setup();
}


void Projector::cameraBegin() {
    camera.begin(view);
}
void Projector::cameraEnd() {
     camera.end();
}


void Projector::fboBegin() {
    fbo.begin();
    
}
void Projector::fboEnd() {
    fbo.end();
}

void Projector::fboBind() {
    fboTexture.bind();
}
void Projector::fboUnbind() {
    fboTexture.unbind();
}

void Projector::draw() {
        
    plane.mapTexCoordsFromTexture(fboTexture);
    ofMatrix4x4 mat = keystone.getMatrix();
    //plane.setTransformMatrix(mat.getPtr() );
    //grid.plane.mapTexCoordsFromTexture(fboTexture);
    
    ofTranslate(width * index + width/2, height/2, 0);
    
    //plane.setPosition(width * index + width/2, height/2, 0);
    
    glEnable(GL_CULL_FACE);
    glCullFace( GL_FRONT );
    //plane.draw();
    //grid.mesh = plane.getMesh();
    
    grid.draw();

    glDisable(GL_CULL_FACE);
    ofTranslate(-(width * index + width/2), -height/2, 0);
}

void Projector::drawWireframe() {
    plane.mapTexCoordsFromTexture(fboTexture);    
    plane.setPosition(width * index + width/2, height/2, 0);    
    plane.drawWireframe();
}


// converts spherical to cartesian coordinates
ofVec3f Projector::sphToCar(ofVec3f t) {
    float azi, ele, dis;
    float x, y, z;
    azi = ofDegToRad(t.x);
    ele = ofDegToRad(t.y+90);
    dis = t.z;
    x = sin(azi) * sin(ele) * dis;
    y = cos(ele) * dis;
    z = cos(azi) * sin(ele) * dis;
    return ofVec3f(x,y,z);
};


// mouse events

void Projector::mousePressed(ofMouseEventArgs& mouseArgs) {
    if (!mouse) {
        return;
    }
    keystone.onMousePressed(mouseArgs);
    grid.onMousePressed(mouseArgs);
}

void Projector::mouseDragged(ofMouseEventArgs& mouseArgs) {
    if (!mouse) {
        return;
    }
    keystone.onMouseDragged(mouseArgs);
    grid.onMouseDragged(mouseArgs);
}

void Projector::mouseReleased(ofMouseEventArgs& mouseArgs) {
    if (!mouse) {
        return;
    }
    grid.onMouseReleased(mouseArgs);
}


// keyboard events

void Projector::keyPressed(int key) {
    switch (key) {
        case 161: // ~ = select all projectors
            keyboard = true;
            break;
    }
    
    if (!keyboard) {
        return;
    }
    
    grid.keyPressed(key);
    
    switch (key) {
               
        case OF_KEY_UP:  // up = switch on mode
            switch (editMode) {                    
                case 1: // projector elevation / distance (super)
                    if (!superKey)
                        elevation += value;
                    else
                        distance += value;
                    setup();
                    break;
                    
                case 2: // projector tilt
                        tilt += value;
                        setup();
                    break;
                    
                case 3: // projector lensOffsetY
                        lensOffsetY += value * .1;
                        setup();
                    break;
                    
                case 4: // projector fov
                        fov += value;
                        setup();                            
                    break;
                    
                case 8: // projector brightness
                        brightness += value * .1;
                    break;
                    
                case 9: // projector contrast
                        contrast += value * .1;
                    break;
                    
                case 10: // projector saturation
                        saturation += value * .1;
                    break;
                    
                case 5: // projector saturation
                        scale += value * .1;
                    break;
            }
            break;
            
            
        case OF_KEY_DOWN: // down = switch on mode
            
            switch (editMode) {

                case 1: // projector elevation / distance (super)
                    if (!superKey)
                        elevation -= value;
                    else
                        distance -= value;
                    setup();
                    break;
                    
                    
                case 2: // projector tilt
                    tilt -= value;
                    setup();
                    break;
                    
                case 3: // projector lensOffsetY
                    lensOffsetY -= value * .1;
                    setup();
                    break;
                    
                case 4: // projector fov
                    fov -= value;
                    setup();
                    break;
                    
                case 8: // projector brightness
                    brightness -= value * .1;
                    break;
                    
                case 9: // projector contrast
                    contrast -= value * .1;
                    break;
                    
                case 10: // projector saturation
                    saturation -= value * .1;
                    break;
                    
                case 5: // projector saturation
                    scale -= value * .1;
                    break;
            }
            break;
            
            
            
            
            
            
        case OF_KEY_LEFT:  // left
            
            switch (editMode) {                    
                case 1: // projector azimuth
                    azimuth -= value;
                    setup();
                break;
                    
                case 2: // projector roll / pan (super)
                    if (!superKey)
                        roll -= value;
                    else
                        pan -= value;
                    setup();
                    break;
                    
                case 3: // projector lensOffsetX
                    lensOffsetX -= value * .1;
                    setup();
                    break;
            }
            break;
            
            
            
            
            
            
        case OF_KEY_RIGHT:  // right
            
            switch (editMode) {
                    
                case 1: // projector azimuth
                    azimuth += value;
                    setup();
                    break;
                    
                case 2: // projector roll / pan (super)
                    if (!superKey)
                        roll += value;
                    else
                        pan += value;
                    setup();
                    break;
                    
                case 3: // projector lensOffsetX
                    lensOffsetX += value * .11;
                    setup();
                    break;
            }
            break;
            
        default:
            break;
    } 
}

void Projector::keyReleased(int key) {
    if (!keyboard) {
        return;
    }
    grid.keyReleased(key);
}



void Projector::loadXML(ofXml &xml) {
    
    string pre = xmlPrefix + ofToString(index);
    
    if (xml.exists(pre + "][@azimuth]"))
        azimuth = ofToFloat( xml.getAttribute(pre + "][@azimuth]") );
    if (xml.exists(pre + "][@brightness]"))
        brightness = ofToFloat( xml.getAttribute(pre + "][@brightness]") );
    if (xml.exists(pre + "][@contrast]"))
        contrast = ofToFloat( xml.getAttribute(pre + "][@contrast]") );
    if (xml.exists(pre + "][@distance]"))
        distance = ofToFloat( xml.getAttribute(pre + "][@distance]") );
    if (xml.exists(pre + "][@elevation]"))
        elevation = ofToFloat( xml.getAttribute(pre + "][@elevation]") );
    if (xml.exists(pre + "][@fov]"))
        fov = ofToFloat( xml.getAttribute(pre + "][@fov]") );
    if (xml.exists(pre + "][@height]"))
        height = ofToInt( xml.getAttribute(pre + "][@height]") );
    if (xml.exists(pre + "][@lensOffsetX]"))
        lensOffsetX = ofToFloat( xml.getAttribute(pre + "][@lensOffsetX]") );
    if (xml.exists(pre + "][@lensOffsetY]"))
        lensOffsetY = ofToFloat( xml.getAttribute(pre + "][@lensOffsetY]") );
    if (xml.exists(pre + "][@saturation]"))
        saturation = ofToFloat( xml.getAttribute(pre + "][@saturation]") );
    if (xml.exists(pre + "][@roll]"))
        roll = ofToFloat( xml.getAttribute(pre + "][@roll]") );
    if (xml.exists(pre + "][@tilt]"))
        tilt = ofToFloat( xml.getAttribute(pre + "][@tilt]") );
    if (xml.exists(pre + "][@pan]"))
        pan = ofToFloat( xml.getAttribute(pre + "][@pan]") );
    if (xml.exists(pre + "][@width]"))
        width = ofToInt( xml.getAttribute(pre + "][@width]") );
    if (xml.exists(pre + "][@scale]"))
        scale = ofToFloat( xml.getAttribute(pre + "][@scale]") );
    
    string str;
    
    if (xml.exists(pre + "][@topLeft]")) {
        str = xml.getAttribute(pre + "][@topLeft]");
        ofStringReplace(str, " ", "");
        topLeft.set( ofToFloat( ofSplitString(str, ",")[0] ), ofToFloat( ofSplitString(str, ",")[1] ) );
    }
    
    if (xml.exists(pre + "][@topRight]")) {
        str = xml.getAttribute(pre + "][@topRight]");
        ofStringReplace(str, " ", "");
        topRight.set( ofToFloat( ofSplitString(str, ",")[0] ), ofToFloat( ofSplitString(str, ",")[1] ) );
    }
    
    if (xml.exists(pre + "][@bottomLeft]")) {
        str = xml.getAttribute(pre + "][@bottomLeft]");
        ofStringReplace(str, " ", "");
        bottomLeft.set( ofToFloat( ofSplitString(str, ",")[0] ), ofToFloat( ofSplitString(str, ",")[1] ) );
    }
    
    if (xml.exists(pre + "][@bottomRight]")) {
        str = xml.getAttribute(pre + "][@bottomRight]");
        ofStringReplace(str, " ", "");
        bottomRight.set( ofToFloat( ofSplitString(str, ",")[0] ), ofToFloat( ofSplitString(str, ",")[1] ) );
    }
   
    
    setup();
}


void Projector::saveXML(ofXml &xml) {
    string pre = xmlPrefix + ofToString(index);
    
    int x = width * index;
    int y = 0;
    
    xml.setAttribute(pre + "][@azimuth]", ofToString(azimuth));
    xml.setAttribute(pre + "][@brightness]", ofToString(brightness));
    xml.setAttribute(pre + "][@contrast]", ofToString(contrast));
    xml.setAttribute(pre + "][@distance]", ofToString(distance));
    xml.setAttribute(pre + "][@elevation]", ofToString(elevation));
    xml.setAttribute(pre + "][@fov]", ofToString(fov));
    xml.setAttribute(pre + "][@height]", ofToString(height));
    xml.setAttribute(pre + "][@lensOffsetX]", ofToString(lensOffsetX));
    xml.setAttribute(pre + "][@lensOffsetY]", ofToString(lensOffsetY));
    xml.setAttribute(pre + "][@saturation]", ofToString(saturation));
    xml.setAttribute(pre + "][@roll]", ofToString(roll));
    xml.setAttribute(pre + "][@tilt]", ofToString(tilt));
    xml.setAttribute(pre + "][@pan]", ofToString(pan));
    xml.setAttribute(pre + "][@width]", ofToString(width));
    xml.setAttribute(pre + "][@scale]", ofToString(scale));

    xml.setAttribute(pre + "][@topLeft]", ofToString( ((keystone.dstPoints[0].x) - x) / width) + "," + ofToString( (keystone.dstPoints[0].y) / height) );
    xml.setAttribute(pre + "][@topRight]", ofToString( ((keystone.dstPoints[1].x) - x) / width) + "," + ofToString( (keystone.dstPoints[1].y) / height) );
    xml.setAttribute(pre + "][@bottomLeft]", ofToString( ((keystone.dstPoints[3].x) - x) / width) + "," + ofToString( (keystone.dstPoints[3].y) / height) );
    xml.setAttribute(pre + "][@bottomRight]", ofToString( ((keystone.dstPoints[2].x) - x) / width) + "," + ofToString( (keystone.dstPoints[2].y) / height) );

}