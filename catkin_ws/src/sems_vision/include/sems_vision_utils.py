import math

def map(x, in_min, in_max, out_min, out_max):
    return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

def mapRectangle(srcShape, destShape, rect):
    srcHeight, srcWidth = (srcShape[0], srcShape[1])
    destHeight, destWidth = (destShape[0], destShape[1])
    x, y, w, h = rect
    leftUpCorner = (map(x, 0, srcWidth, 0, destWidth), map(y, 0, srcWidth, 0, destWidth))
    rightDownCorner = (map(x + w, 0, srcWidth, 0, destWidth), map(y + h, 0, srcWidth, 0, destWidth))
    return (leftUpCorner[0], leftUpCorner[1], rightDownCorner[0] - leftUpCorner[0], rightDownCorner[1] - leftUpCorner[1])

def add_image(background, image, position):
    y1, y2 = position[1], position[1] + image.shape[0]
    x1, x2 = position[0], position[0] + image.shape[1]
    alpha_s = image[:, :, 3] / 255.0
    alpha_l = 1.0 - alpha_s

    for c in range(0, 3):
        background[y1:y2, x1:x2, c] = (alpha_s * image[:, :, c] +
                                alpha_l * background[y1:y2, x1:x2, c])
    return background

def get_depth(rgbframe_, depthframe_, pixel):
    heightRGB, widthRGB = (rgbframe_.shape[0], rgbframe_.shape[1])
    heightDEPTH, widthDEPTH = (depthframe_.shape[0], depthframe_.shape[1])

    
    x = map(pixel[0], 0, widthRGB, 0, widthRGB)
    y = map(pixel[1], 0, widthDEPTH, 0, widthDEPTH)

    def medianCalculation(x, y, width, height, depthframe_):
        medianArray = []
        requiredValidValues = 20
        def spiral(medianArray, depthframe_, requiredValidValues, startX, startY, endX, endY, width, height):
            if startX <  0 and startY < 0 and endX > width and endY > height:
                return
            for i in range(startX, endX + 1):
                if i >= width:
                    break
                if startY >= 0 and math.isfinite(depthframe_[startY][i]):
                    medianArray.append(depthframe_[startY][i])
                if startY != endY and endY < height and math.isfinite(depthframe_[endY][i]):
                    medianArray.append(depthframe_[endY][i])
                if len(medianArray) > requiredValidValues:
                    return
            for i in range(startY + 1, endY):
                if i >= height:
                    break
                if startX >= 0 and math.isfinite(depthframe_[i][startX]):
                    medianArray.append(depthframe_[i][startX])
                if startX != endX and endX < width and math.isfinite(depthframe_[i][endX]):
                    medianArray.append(depthframe_[i][endX])
                if len(medianArray) > requiredValidValues:
                    return
            # Check Next Spiral
            spiral(medianArray, depthframe_, requiredValidValues, startX - 1, startY - 1, endX + 1, endY + 1, width, height)
        
        # Check Spirals around Centroid till requiredValidValues
        spiral(medianArray, depthframe_, requiredValidValues, x, y, x, y, width, height)
        if len(medianArray) == 0:
            return float("NaN")
        medianArray.sort()
        return medianArray[len(medianArray) // 2]
    
    return medianCalculation(x, y, widthDEPTH, heightDEPTH, depthframe_)

def deproject_pixel_to_point(cv_image_rgb_info, pixel, depth):
    def CameraInfoToIntrinsics(cameraInfo):
        intrinsics = {}
        intrinsics["width"] = cameraInfo.width
        intrinsics["height"] = cameraInfo.height
        intrinsics["ppx"] = cameraInfo.K[2]
        intrinsics["ppy"] = cameraInfo.K[5]
        intrinsics["fx"] = cameraInfo.K[0]
        intrinsics["fy"] = cameraInfo.K[4]
        if cameraInfo.distortion_model == 'plumb_bob':
            intrinsics["model"] = "RS2_DISTORTION_BROWN_CONRADY"
        elif cameraInfo.distortion_model == 'equidistant':
            intrinsics["model"] = "RS2_DISTORTION_KANNALA_BRANDT4"
        intrinsics["coeffs"] = [i for i in cameraInfo.D]
        return intrinsics
    
    intrinsics = CameraInfoToIntrinsics(cv_image_rgb_info)

    if(intrinsics["model"] == "RS2_DISTORTION_MODIFIED_BROWN_CONRADY"): # Cannot deproject from a forward-distorted image
        return

    x = (pixel[0] - intrinsics["ppx"]) / intrinsics["fx"]
    y = (pixel[1] - intrinsics["ppy"]) / intrinsics["fy"]

    xo = x
    yo = y

    if (intrinsics["model"] == "RS2_DISTORTION_INVERSE_BROWN_CONRADY"):
        # need to loop until convergence 
        # 10 iterations determined empirically
        for i in range(10):
            r2 = float(x * x + y * y)
            icdist = float(1) / float(1 + ((intrinsics["coeffs"][4] * r2 + intrinsics["coeffs"][1]) * r2 + intrinsics["coeffs"][0]) * r2)
            xq = float(x / icdist)
            yq = float(y / icdist)
            delta_x = float(2 * intrinsics["coeffs"][2] * xq * yq + intrinsics["coeffs"][3] * (r2 + 2 * xq * xq))
            delta_y = float(2 * intrinsics["coeffs"][3] * xq * yq + intrinsics["coeffs"][2] * (r2 + 2 * yq * yq))
            x = (xo - delta_x) * icdist
            y = (yo - delta_y) * icdist

    if intrinsics["model"] == "RS2_DISTORTION_BROWN_CONRADY":
        # need to loop until convergence 
        # 10 iterations determined empirically
        for i in range(10):
            r2 = float(x * x + y * y)
            icdist = float(1) / float(1 + ((intrinsics["coeffs"][4] * r2 + intrinsics["coeffs"][1]) * r2 + intrinsics["coeffs"][0]) * r2)
            delta_x = float(2 * intrinsics["coeffs"][2] * x * y + intrinsics["coeffs"][3] * (r2 + 2 * x * x))
            delta_y = float(2 * intrinsics["coeffs"][3] * x * y + intrinsics["coeffs"][2] * (r2 + 2 * y * y))
            x = (xo - delta_x) * icdist
            y = (yo - delta_y) * icdist

    if intrinsics["model"] == "RS2_DISTORTION_KANNALA_BRANDT4":
        rd = float(math.sqrt(x * x + y * y))
        if rd < FLT_EPSILON:
            rd = FLT_EPSILON

        theta = float(rd)
        theta2 = float(rd * rd)
        for i in range(4):
            f = float(theta * (1 + theta2 * (intrinsics["coeffs"][0] + theta2 * (intrinsics["coeffs"][1] + theta2 * (intrinsics["coeffs"][2] + theta2 * intrinsics["coeffs"][3])))) - rd)
            if fabs(f) < FLT_EPSILON:
                break
            df = float(1 + theta2 * (3 * intrinsics["coeffs"][0] + theta2 * (5 * intrinsics["coeffs"][1] + theta2 * (7 * intrinsics["coeffs"][2] + 9 * theta2 * intrinsics["coeffs"][3]))))
            theta -= f / df
            theta2 = theta * theta
        r = float(math.tan(theta))
        x *= r / rd
        y *= r / rd

    if intrinsics["model"] == "RS2_DISTORTION_FTHETA":
        rd = float(math.sqrt(x * x + y * y))
        if rd < FLT_EPSILON:
            rd = FLT_EPSILON
        r = (float)(math.tan(intrinsics["coeffs"][0] * rd) / math.atan(2 * math.tan(intrinsics["coeffs"][0] / float(2.0))))
        x *= r / rd
        y *= r / rd

    return (depth * x, depth * y, depth)

def calculatedistance(point1, point2):
    return  math.sqrt(
                math.pow(point1[0] - point2[0], 2) + math.pow(point1[1] - point2[1], 2) + math.pow(
                    point1[2] - point2[2], 2))

def get_optimal_font_scale(text, width, font, thickness):
    for scale in reversed(range(0, 60)):
        textSize = cv2.getTextSize(text, fontFace=font, fontScale=scale/10, thickness=thickness)
        (new_width, new_height) = textSize[0]
        if (new_width <= width):
            return (scale/10, new_height)
    return (1,1)

def point_inside_rect(point, rect) :
    x, y, w, h = rect
    if point[0] > x and point[0] < x + w and point[1] > y and point[1] < y + h:
        return True
    else:
        return False
