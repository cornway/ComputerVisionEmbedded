# STM32 computer vision models post-processing

This readme explains how to use the different available post-processing code. 


## Available post-processings


| Models        | Task                 |
|---------------|----------------------|
| ST YOLOX      | person_detection     |
| YOLOv8        | person_detection     |
| YOLOv5        | person_detection     |
| YOLOv4        | person_detection     |
| Tiny Yolo V2  | person_detection     |
| Standard SSD  | person_detection     |
| ST SSD        | person_detection     |
| Centernet     | person_detection     |
| Blazeface     | face_detection       |
| YOLOv8 Seg    | instance_segmentation |
| Deeplabv3     | semantic_segmentation |
| YOLOv8 Pose   | mpe_estimation       |
| MoveNet       | spe_estimation       |
| CNN_pd     | palm_detection |

## Version History

### v0.10.0 - 2025-07-10

- **Improvements:**
  - Blazeface face detection post-processing uint8, int8 and float32_t

### v0.9.0 - 2025-04-29

- **Improvements:**
  - int8 input for post-processing (model output) available for all post-processing
  - YOLOv4 object detection post-processing quantization per tensor
  - Updated README.md

### v0.8.0 - 2025-04-01

- **New Features:**
  - Added support for YOLOv4 object detection post-processing.

### v0.7.3 - 2025-03-24

- **Improvements:**
  - st_yolox_pp optimization for 1 class case.
  - remove some unused code

### v0.7.2 - 2024-11-19

- **Improvements:**
  - removed some old unused verification application

### v0.7.1 - 2024-11-19
- **Bug Fixes:**
  - Fixed MVEI vision_models_maxi_tr_p_is8ou8 parameters (using uint32_t for parallelize to avoid truncation)

### v0.7.0 - 2024-11-15

- **New Features:**
  - Added support for palm detection post-processing.
  - Added support for ST YOLOX object detection post-processing.
- **Improvements:**
  - Move some parameters from output or input structures to parameters structure.
  - Improve performances for instance segmentation YOLOv8 post-processing (data used and cycles).
  - Added int8_t semantic segmentation post-processing (along with existing uint8_t and float32)
- **Bug Fixes:**
  - Fixed some compilation warnings.

### v0.6.0 - 2024-10-28

- **New Features:**
  - Added support for instance segmentation YOLOv8 model post-processing.
  - Added support for multi-pose estimation YOLOv8 model post-processing.
  - Added support for single-pose estimation MoveNet model post-processing.
  - Added support for semantic segmentation DeepLabv3 model post-processing.
- **Improvements:**
  - Renamed for better maintainability.

### v0.5.0 - 2024-07-30

- **Initial Release:**
  - Support for object-detection SSD model post-processing.
  - Support for object-detection SSD_ST model post-processing.
  - Support for object-detection YOLOlc model post-processing.
  - Support for object-detection YOLOv5 model post-processing.
  - Support for object-detection YOLOv8 model post-processing.


# Post-Processing Output Structures
<details>

## Object detection post-processing Output Structures

<details>

---
### `od_pp_outBuffer_t`

This structure represents the output buffer for a single object detection result. It contains information about the detected object's position, size, confidence score, and class index.

Parameters:

- **float32_t x_center**: The normalized x-coordinate of the center of the detected object.
- **float32_t y_center**: The normalized y-coordinate of the center of the detected object.
- **float32_t width**: The normalized width of the detected object.
- **float32_t height**: The normalized height of the detected object.
- **float32_t conf**: The confidence (between 0.0 and 1.0) score of the detection.
- **int32_t class_index**: The index of the detected object's class.

---
### `od_pp_out_t`

This structure represents the overall output of object detection post-processing step. It contains a pointer to an array of od_pp_outBuffer_t structures and the number of detections.

Parameters:

- **od_pp_outBuffer_t \*pOutBuff**: Pointer to an array of od_pp_outBuffer_t structures.
- **int32_t nb_detect**: The number of detections in the output buffer.

---

</details>

## Instance segmentation post-processing Output Structures

<details>

---
### `iseg_pp_outBuffer_t`

This structure represents the output buffer for a single instance segmantion result. It contains information about the detected object's position, size, confidence score, class index and mask.

Parameters:

- **float32_t x_center**: The normalized x-coordinate of the center of the object.
- **float32_t y_center**: The normalized y-coordinate of the center of the object.
- **float32_t width**: The normalized width of the object.
- **float32_t height**: The normalized height of the object.
- **float32_t conf**: The confidence (between 0.0 and 1.0) score of the detection.
- **int32_t class_index**: The index of the object's class.
- **uint8_t \*pMask**: The pointer to the mask buffer which has AI_ISEG_YOLOV8_PP_MASK_SIZE * AI_SEG_YOLOV8_PP_MASK_SIZE elements
---
### `iseg_pp_out_t`

This structure is designed to encapsulate the output data from a segmentation post-processing step in an object detection pipeline. This structure includes pointers to the detected objects' information, their segmentation masks, and the number of detected objects.

Parameters:

- **iseg_pp_outBuffer_t \*pOutBuff**: Pointer to an array of postprocess_outBuffer_t structures.
- **int32_t nb_detect**: The number of detections in the output buffer.
---

</details>

## Multi-pose estimation post-processing Output Structures

<details>

---
### `mpe_pp_keyPoints_t`

This structure is used to represent key points in a post-processing context. Each key point consists of an x-coordinate, a y-coordinate, and a confidence score.

Parameters:

- **float32_t x**: The normalized x-coordinate of the keypoint.
- **float32_t y**: The normalized y-coordinate of the keypoint.
- **float32_t conf**: The visibility (between 0.0 and 1.0) score of the keypoint.

---
### `mpe_pp_outBuffer_t`

This structure represents the output buffer for a single multi_pose estimation result. It contains information about the detected object's position, size, confidence score, class index and key points.

Parameters:

- **float32_t x_center**: The normalized x-coordinate of the center of the detected object.
- **float32_t y_center**: The normalized y-coordinate of the center of the detected object.
- **float32_t width**: The normalized width of the detected object.
- **float32_t height**: The normalized height of the detected object.
- **float32_t conf**: The confidence (between 0.0 and 1.0) score of the detection.
- **int32_t class_index**: The index of the detected object's class.
- **mpe_pp_keyPoints_t \*pKeyPoints**: The pointer to the array of key points (which size is AI_MPE_YOLOV8_PP_KEYPOINTS_NB).

---
### `mpe_pp_out_t`

This structure represents the overall output of the multi-pose estimation post-processing step. It contains a pointer to an array of mpe_pp_outBuffer_t structures and the number of detections.

Parameters:

- **mpe_pp_outBuffer_t \*pOutBuff**: Pointer to an array of mpe_pp_outBuffer_t structures.
- **int32_t nb_detect**: The number of detections in the output buffer.

</details>

## Single-pose estimation post-processing Output Structures

<details>

---
### `spe_pp_outBuffer_t`

This structure is used to represent key points in a post-processing context. Each key point consists of an x-coordinate, a y-coordinate, and a confidence score.

Parameters:

- **float32_t x**: The normalized x-coordinate of the keypoint.
- **float32_t y**: The normalized y-coordinate of the keypoint.
- **float32_t proba**: The visibility (between 0.0 and 1.0) score of the keypoint.

---
### `spe_pp_out_t`

This structure represents the overall output of the single-pose estimation post-processing step. It contains a pointer to an array of spe_pp_outBuffer_t structures.

Parameters:

- **spe_pp_outBuffer_t \*pOutBuff**: Pointer to an array of spe_pp_outBuffer_t structure (dimension is number of keypoints).

</details>

## Semantic Segmentation post-processing Output Structures

<details>
---

### `sseg_pp_out_t`

This structure represents the overall output of the semantic segmentation post-processing step. It contains a pointer to an array of uint8_t.

Parameters:

- **uint8_t \*pOutBuff**: Pointer to an map array of uint8_t identifiying classes (dimension height*width).

</details>

## Palm detection post-processing Output Structures

<details>

---
### `pd_pp_point_t`

This structure represents one keypoint. It contains information about the detected key's position.

Parameters:

- **float32_t x**: The normalized x-coordinate of the keypoint.
- **float32_t y**: The normalized y-coordinate of the keypoint.

---
### `pd_pp_box_t`

This structure represents the output buffer for a single palm detection result. It contains information about the detected object's position, size, confidence score, and keypoints.

Parameters:

- **float32_t prob**: The confidence (between 0.0 and 1.0) score of the detection.
- **float32_t x_center**: The normalized x-coordinate of the center of the detected object.
- **float32_t y_center**: The normalized y-coordinate of the center of the detected object.
- **float32_t width**: The normalized width of the detected object.
- **float32_t height**: The normalized height of the detected object.
- **pd_pp_point_t \*pKps**: The pointer to the keypoints coordinates (buffer should be AI_PD_MODEL_PP_NB_KEYPOINTS pd_pp_point_t elements).

---
### `pd_pp_out_t`

This structure represents the overall output of object detection post-processing step. It contains a pointer to an array of pd_pp_box_t structures and the number of detections.

Parameters:

- **pd_pp_box_t \*pOutData**: Pointer to an array of pd_pp_box_t structures (pointed buffer should be at least AI_PD_MODEL_PP_MAX_BOXES_LIMIT * sizeof(pd_pp_box_t).
- **uint32_t box_nb**: The number of detections in the output buffer.

---

</details>

</details>

# Object Detection Post-Processings

<details>

# YOLOV8 Object Detection Post-Processing

<details>

## YOLOv8 Structures
---
### `od_yolov8_pp_in_centroid_t`

This structure is used for YOLOv8 post-processing input where the raw detections are in float32_t/int8_t format.

Parameters:

- **void_t \*pRaw_detections**: Pointer to raw detection data in float32_t format.
---
### `od_yolov8_pp_static_param_t`

This structure holds the static parameters required for YOLOv8 post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_total_boxes**: Total number of boxes predicted by the model. To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **float32_t raw_output_scale**: Scale factor for raw output values.
- **int8_t raw_output_zero_point**: Zero point for quantized raw output values.
- **int32_t nb_detect**: Number of detections after post-processing.
- **void \*pScratchBuff**: pointer to scratch buffer whose size is AI_OD_YOLOV8_PP_TOTAL_BOXES * 6 * sizeof(int8_t)
---
## YOLOv8 Routines
---
### Pointer initialization

Scratch buffer pointer in od_yolov8_pp_static_param_t when using int8_t input data to reduce memory usage:
```c
int8_t scratch_buffer[AI_OD_YOLOV8_PP_TOTAL_BOXES*(4+2)];
```

```c
od_yolov8_pp_static_param_t y8_input_param;

y8_input_param.pScratchBuff = scratch_buffer;

```

### `od_yolov8_pp_reset`

**Purpose**:  
Resets the static parameters for YOLOv8 post-processing.

**Prototype**:  
```c
int32_t od_yolov8_pp_reset(od_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the YOLOv8 post-processing by setting the number of detected objects to zero.

---

### `od_yolov8_pp_process`

**Purpose**:  
Processes the YOLOv8 post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t od_yolov8_pp_process(od_yolov8_pp_in_centroid_t *pInput,
                             od_pp_out_t *pOutput,
                             od_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function performs the post-processing steps for YOLOv8 object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `od_yolov8_pp_process_int8`

**Purpose**:  
Processes the YOLOv8 post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t od_yolov8_pp_process_int8(od_yolov8_pp_in_centroid_t *pInput,
                                  od_pp_out_t *pOutput,
                                  od_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv8 object detection with int8_t input data. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_OD_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

# YOLOV5 Object Detection Post-Processing
<details>

## YOLOv5 Structures
---
### `od_yolov5_pp_in_centroid_t`

This structure is used for YOLOv5 post-processing input where the raw detections are in float32_t/uint8_t format.

Parameters:

- **void \*pRaw_detections**: Pointer to raw detection data in float32_t/uint8_t format.
---
### `od_yolov5_pp_static_param_t`

This structure holds the static parameters required for YOLOv5 post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_total_boxes**: Total number of boxes predicted by the model. To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **float32_t raw_output_scale**: Scale factor for model quantized raw output values.
- **int8_t raw_output_zero_point**: Zero point for moldel quantized raw output values.
- **int32_t nb_detect**: Number of detections after post-processing.
---
## YOLOv5 Routines
---
### `od_yolov5_pp_reset`

**Purpose**:  
Resets the static parameters for YOLOv5 post-processing.

**Prototype**:  
```c
int32_t od_yolov5_pp_reset(yolov5_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the YOLOv5 post-processing by setting the number of detected objects to zero.

---

### `od_yolov5_pp_process`

**Purpose**:  
Processes the YOLOv5 post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t od_yolov5_pp_process(od_yolov5_pp_in_centroid_t *pInput,
                             od_pp_out_t *pOutput,
                             od_yolov5_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv5 object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

#### `od_yolov5_pp_process_uint8`

**Purpose**:  
Processes the YOLOv5 post-processing pipeline for uint8_t input data.

**Prototype**:  
```c
int32_t od_yolov5_pp_process_uint8(od_yolov5_pp_in_centroid_t *pInput,
                                   od_pp_out_t *pOutput,
                                   od_yolov5_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv5 object detection with uint8_t input data. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_OD_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>


# YOLOV4 Object Detection Post Processing
<details>

## YOLOv4 Structures
---
### `od_yolov4_pp_in_centroid_t`

This structure is used for YOLOv4 post-processing input where the raw detections are in float32/int8 format.

Parameters:

- **void *\*pRaw_boxes**: Pointer to raw detection boxes in float32/int8 format (output of the model)
- **void *\*pRaw_probas**: Pointer to raw detection probas in float32/int8 format (output of the model).
---

### `od_yolov4_pp_static_param_t`

This structure holds the static parameters required for YOLOv4 post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_total_boxes**: Total number of boxes predicted by the model. To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **float32_t proba_scale**: Scale factor for proba quantized tensor values.
- **float32_t boxe_scale**: Scale factor for boxe quantized tensor values.
- **int32_t nb_detect**: Number of detections after post-processing.
- **void *pScratchBuff**: optional scratch buffer when int8 inputs processing (size is: nb_total_boxes * 6 * int8 (4 for boxes coordinates, and 2 for confidence and classe id))
                          if unused, set to NULL.
- **int8_t proba_zero_point**: Zero point for proba quantized tensor values.
- **int8_t boxe_zero_point**: Zero point for boxe quantized tensor values.

---
## YOLOv4 Routines
---
### `od_yolov4_pp_reset`

**Purpose**:  
Resets the static parameters for YOLOv4 post-processing.

**Prototype**:  
```c
int32_t od_yolov4_pp_reset(od_yolov4_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the YOLOv4 post-processing by setting the number of detected objects to zero.

---

### `od_yolov4_pp_process`

**Purpose**:  
Processes the YOLOv4 post-processing pipeline for float32 input data.

**Prototype**:  
```c
int32_t od_yolov4_pp_process(od_yolov4_pp_in_centroid_t *pInput,
                             od_pp_out_t *pOutput,
                             od_yolov4_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the yolov4 input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- AI_OD_POSTPROCESS_ERROR_NO on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv4 object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

#### `od_yolov4_pp_process_int8`

**Purpose**:  
Processes the YOLOv4 post-processing pipeline for int8 input data.

**Prototype**:  
```c
int32_t od_yolov4_pp_process_uint8(od_yolov4_pp_in_centroid_t *pInput,
                                   od_pp_out_t *pOutput,
                                   od_yolov4_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the yolov4 input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv4 object detection with uint8 input data. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.

---

</details>

# Tiny YOLOV2 Object Detection Post Processing
<details>

## Tiny YOLOV2 Structures
---
### `od_yolov2_pp_in_t`

This structure is used for Tiny YOLOV2 post-processing input where the raw detections are in float32_t/int8_t format.

Parameters:

- **void \*pRaw_detections**: Pointer to raw detection data in float32_t/int8_t format.
---
### `od_yolov2_pp_static_param_t`

This structure holds the static parameters required for Tiny YOLOV2 post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_anchors**: Number of the anchors used by the model. To extract fom the model output shape.
- **int32_t grid_width**: The width of the model output. To extract fom the model output shape.
- **int32_t grid_height**: The height of the model output. To extract fom the model output shape.
- **int32_t nb_input_boxes**: Total number of boxes predicted by the model. nb_input_boxes = nb_anchors x grid_width x grid_height
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **int32_t nb_detect**: Number of detections after post-processing.
- **const float32_t \*pAnchors**: A pointer to an array of anchor box dimensions. Each anchor box is defined by its width and height. The array should have a length of 2 x nb_anchors, where each pair of values represents the width and height of an anchor box.
- **float32_t raw_output_scale**: Scale factor for model quantized raw output values.
- **int8_t raw_output_zero_point**: Zero point for moldel quantized raw output values.
- **void \*pScratchBuffer**: pointer to scratch buffer used for temporary data (AI_OD_YOLOV2_PP_GRID_WIDTH * AI_OD_YOLOV2_PP_GRID_HEIGHT * AI_OD_YOLOV2_PP_NB_ANCHORS * sizeof(od_pp_outBuffer_t)). If set to NULL, and sizeof input (i.e. AI_YOLOV2_PP_CLASSPROB + AI_OD_YOLOV2_PP_NB_CLASSES * sizeof(<input_data>) >= sizeof(od_pp_outBuffer_t)), would be overlayed with input data buffer.

---
## Tiny YOLOV2 Routines
---
### `od_yolov2_pp_reset`

**Purpose**:  
Resets the static parameters for Tiny YOLOV2 post-processing.

**Prototype**:  
```c
int32_t od_yolov2_pp_reset(od_yolov2_pp_static_param_t *pInput_static_param);

```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the Tiny YOLOV2 post-processing by setting the number of detected objects to zero.

---

### `od_yolov2_pp_process`

**Purpose**:  
Processes the Tiny YOLOV2 post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t od_yolov2_pp_process(od_yolov2_pp_in_t *pInput,
                             od_pp_out_t *pOutput,
                             od_yolov2_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for Tiny YOLOV2 object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `od_yolov2_pp_process_int8`

**Purpose**:  
Processes the Tiny YOLOV2 post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t od_yolov2_pp_process_int8(od_yolov2_pp_in_t *pInput,
                                  od_pp_out_t *pOutput,
                                  od_yolov2_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for Tiny YOLOV2 object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_OD_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

# Standard SSD Object Detection Post-Processing
<details>

## Standard SSD Structures
---
### `od_ssd_pp_in_centroid_t`

This structure is used for Standard SSD post-processing input where the raw detections are in float32_t/int8_t format.

Parameters:

- **void \*pBoxes**: Pointer to the raw Boxes data in float32_t/int8_t format.
- **void \*pAnchors**: Pointer to the Anchors data in float32_t/int8_t format.
- **void \*pScores**: Pointer to the Scores data in float32_t/int8_t format.
---
### `od_ssd_pp_static_param_t`

This structure holds the static parameters required for Standard SSD post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **float32_t XY_scale**: Scale factor applied to the XY coordinates of the bounding boxes. To extract fom the tflite post-processing layer (before removing it from the model).
- **float32_t WH_scale**: Scale factor applied to the width and height of the bounding boxes. To extract fom the tflite post-processing layer (before removing it from the model).
- **int32_t nb_detections**: Total number of boxes predicted by the model. To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **int32_t nb_detect**: Number of detections after post-processing.
- **void \*scratchBuffer**: pointer to a scratch buffer with size AI_OD_SSD_PP_TOTAL_DETECTIONS * sizeof(od_pp_outBuffer_t).  If set to NULL, and sizeof input (i.e. AI_OD_YOLOV2_PP_NB_CLASSES * sizeof(<input_data>) >= sizeof(od_pp_outBuffer_t)), would be overlayed with scores data buffer.
- **float32_t boxe_scale**: Scale factor for model quantized raw output boxes values
- **float32_t anchor_scale**: Scale factor for model quantized raw output anchors values
- **float32_t score_scale**: Scale factor for model quantized raw output scores values
- **int8_t boxe_zero_point**: Zero point for moldel quantized raw output boxes values
- **int8_t anchor_zero_point**: Zero point for moldel quantized raw output anchors values
- **int8_t score_zero_point**: Zero point for moldel quantized raw output scores values

---
## Standard SSD Routines
---
### `od_ssd_pp_reset`

**Purpose**:  
Resets the static parameters for Standard SSD post-processing.

**Prototype**:  
```c
int32_t od_ssd_pp_reset(od_ssd_pp_static_param_t *pInput_static_param);

```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the Standard SSD post-processing by setting the number of detected objects to zero.

---

### `od_ssd_pp_process`

**Purpose**:  
Processes the Standard SSD post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t od_ssd_pp_process(od_ssd_pp_in_centroid_t *pInput, 
                          od_pp_out_t *pOutput, 
                          od_ssd_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for Standard SSD object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `od_ssd_pp_process_int8`

**Purpose**:  
Processes the Standard SSD post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t od_ssd_pp_process_int8(od_ssd_pp_in_centroid_t *pInput, 
                               od_pp_out_t *pOutput, 
                               od_ssd_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for Standard SSD object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_OD_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

# ST SSD Object Detection Post-Processing
<details>

## ST SSD Structures
---
### `od_ssd_st_pp_in_centroid_t`

This structure is used for ST SSD post-processing input where the raw detections are in float32_t/int8_t format.

Parameters:

- **void \*pBoxes**: Pointer to the raw Boxes data in float32_t/int8_t format.
- **void \*pAnchors**: Pointer to the Anchors data in float32_t/int8_t format.
- **void \*pScores**: Pointer to the Scores data in float32_t/int8_t format.
---
### `od_ssd_st_pp_static_param_t`

This structure holds the static parameters required for ST SSD post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_detections**: Total number of boxes predicted by the model. To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **int32_t nb_detect**: Number of detections after post-processing.
- **void \*scratchBuffer**: pointer to a scratch buffer with size AI_OD_SSD_PP_TOTAL_DETECTIONS * sizeof(od_pp_outBuffer_t).  If set to NULL, and sizeof input (i.e. AI_OD_YOLOV2_PP_NB_CLASSES * sizeof(<input_data>) >= sizeof(od_pp_outBuffer_t)), would be overlayed with scores data buffer.
- **float32_t boxe_scale**: Scale factor for model quantized raw output boxes values
- **float32_t anchor_scale**: Scale factor for model quantized raw output anchors values
- **float32_t score_scale**: Scale factor for model quantized raw output scores values
- **int8_t boxe_zero_point**: Zero point for moldel quantized raw output boxes values
- **int8_t anchor_zero_point**: Zero point for moldel quantized raw output anchors values
- **int8_t score_zero_point**: Zero point for moldel quantized raw output scores values
---
## ST SSD Routines
---
### `od_ssd_st_pp_reset`

**Purpose**:  
Resets the static parameters for ST SSD post-processing.

**Prototype**:  
```c
int32_t od_ssd_st_pp_reset(od_ssd_st_pp_static_param_t *pInput_static_param);

```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the ST SSD post-processing by setting the number of detected objects to zero.

---

### `od_ssd_st_pp_process`

**Purpose**:  
Processes the ST SSD post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t od_ssd_st_pp_process(od_ssd_st_pp_in_centroid_t *pInput,
                             od_pp_out_t *pOutput,
                             od_ssd_st_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for ST SSD object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `od_ssd_st_pp_process_int8`

**Purpose**:  
Processes the ST SSD post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t od_ssd_st_pp_process_int8(od_ssd_st_pp_in_centroid_t *pInput,
                                  od_pp_out_t *pOutput,
                                  od_ssd_st_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for ST SSD object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_OD_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>


# ST YOLOX Object Detection Post-Processing
<details>

## ST YOLOX Structures
---
### `od_st_yolox_pp_in_t`

This structure is used for ST YOLOX post-processing input where the raw detections are in float32_t/int8_t format.

Parameters:

- **void \*pRaw_detections_L**: Pointer to the large raw detection data in float32_t/int8_t format.
- **void \*pRaw_detections_M**: Pointer to the medium raw detection data in float32_t/int8_t format.
- **void \*pRaw_detections_S**: Pointer to the small raw detection data in float32_t/int8_t format.
---
### `od_st_yolox_pp_static_param_t`

This structure holds the static parameters required for ST YOLOX post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_anchors**: Number of the anchors used by the model. To extract fom the model output shape.
- **int32_t grid_width_L**: The width of the model large output. To extract fom the model outputs shape.
- **int32_t grid_height_L**: The height of the model large output. To extract fom the model output shape.
- **int32_t grid_width_M**: The width of the model medium output. To extract fom the model outputs shape.
- **int32_t grid_height_M**: The height of the model medium output. To extract fom the model output shape.
- **int32_t grid_width_S**: The width of the model small output. To extract fom the model outputs shape.
- **int32_t grid_height_S**: The height of the model small output. To extract fom the model output shape.
- **int32_t nb_input_boxes**: Total number of boxes predicted by the model. nb_input_boxes = nb_anchors x grid_width x grid_height
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **int32_t nb_detect**: Number of detections after post-processing.
- **const float32_t \*pAnchors_L**: A pointer to the large array of anchor box dimensions. Each anchor box is defined by its width and height. The array should have a length of 2 x nb_anchors, where each pair of values represents the width and height of an anchor box.
- **const float32_t \*pAnchors_M**: A pointer to the medium array of anchor box dimensions. Each anchor box is defined by its width and height. The array should have a length of 2 x nb_anchors, where each pair of values represents the width and height of an anchor box.
- **const float32_t \*pAnchors_S**: A pointer to the small array of anchor box dimensions. Each anchor box is defined by its width and height. The array should have a length of 2 x nb_anchors, where each pair of values represents the width and height of an anchor box.
- **float32_t raw_l_scale: Scale factor for raw model large ouput values (**int8** input data).
- **float32_t raw_m_scale: Scale factor for raw model medium ouput values (**int8** input data).
- **float32_t raw_s_scale: Scale factor for raw model small ouput values (**int8** input data).
- **int8_t raw_l_zero_point: Zero point for raw model large ouput values (**int8** input data).
- **int8_t raw_m_zero_point: Zero point for raw model medium ouput values (**int8** input data).
- **int8_t raw_s_zero_point: Zero point for raw model small ouput values (**int8** input data).
---
## ST YOLOX Routines
---
### `od_st_yolox_pp_reset`

**Purpose**:  
Resets the static parameters for ST YOLOX post-processing.

**Prototype**:  
```c
int32_t od_st_yolox_pp_reset(od_st_yolox_pp_static_param_t *pInput_static_param);

```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the ST YOLOX post-processing by setting the number of detected objects to zero.

---

### `od_st_yolox_pp_process`

**Purpose**:  
Processes the ST YOLOX post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t od_st_yolox_pp_process(od_st_yolox_pp_in_t *pInput,
                               od_pp_out_t *pOutput,
                               od_st_yolox_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for ST YOLOX object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `od_st_yolox_pp_process`

**Purpose**:  
Processes the ST YOLOX post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t od_st_yolox_pp_process_int8(od_st_yolox_pp_in_t *pInput,
                                    od_pp_out_t *pOutput,
                                    od_st_yolox_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for ST YOLOX object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_OD_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

# Blazeface Face-Detection Post-Processing

<details>

## Blazeface Structures
---
### `od_fd_blazeface_pp_in_t`

This structure is used for Blazeface post-processing input where the raw detections are in float32_t/uint8_t format.

Parameters:

- **void_t \*pRaw_detections_0**: Pointer to raw detection data in float32_t/uint8_t/int8_t format.
- **void_t \*pRaw_detections_1**: Pointer to raw detection data in float32_t/uint8_t/int8_t format.
- **void_t \*pScores_0**: Pointer to raw probabilities in float32_t/uint8_t/int8_t format.
- **void_t \*pScores_1**: Pointer to raw probabilities in float32_t/uint8_t/int8_t format.

---
### `od_fd_blazeface_pp_static_param_t`

This structure holds the static parameters required for Blazeface post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_keypoints**: Number of keypoints in raw detections. To extract fom the model output shape.
- **int32_t nb_detections_0**: Total number of boxes predicted first output from the model. To extract fom the model output shape.
- **int32_t nb_detections_1**: Total number of boxes predicted second output from the model. To extract fom the model output shape.
- **int32_t in_size**: size of of the input image of the model (in_size x in_size as squared). To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **int32_t nb_detect**: Number of detections after post-processing. To be resetted before each process.
- **const void *pAnchors_0**: Pointer to anchors for first output. To compute fom the model.
- **const void *pAnchors_1**: Pointer to anchors for second output. To compute fom the model.
- **float32_t boxe_0_scale**: Scale factor for first raw boxes output values.
- **float32_t proba_0_scale**: Scale factor for first scores output values.
- **float32_t boxe_1_scale**: Scale factor for second raw boxes output values.
- **float32_t proba_1_scale**: Scale factor for second scores output values.
- **uint8_t boxe_0_zero_point**: Zero point for quantized first raw boxes output values.
- **uint8_t proba_0_zero_point**: Zero point for quantized first raw probabilities output values.
- **uint8_t boxe_1_zero_point**: Zero point for quantized second raw boxes output values.
- **uint8_t proba_1_zero_point**: Zero point for quantized seconf raw probabilities output values.
---
## Blazeface Routines
---

### `od_fd_blazeface_pp_reset`

**Purpose**:
Resets the static parameters for Blazeface post-processing.

**Prototype**:
```c
int32_t od_fd_blazeface_pp_reset(od_fd_blazeface_pp_static_param_t *pInput_static_param);
```

**Parameters**:
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:
This function initializes the static parameters for the Blazeface post-processing by setting the number of detected objects to zero.

---

### `od_fd_blazeface_pp_process`

**Purpose**:
Processes the Blazeface post-processing pipeline for float32_t input data.

**Prototype**:
```c
int32_t od_fd_blazeface_pp_process(od_fd_blazeface_pp_in_t *pInput,
                                   od_pp_out_t *pOutput,
                                   od_fd_blazeface_pp_static_param_t *pInput_static_param);
```

**Parameters**:
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:
- **AI_OD_POSTPROCESS_ERROR_NO** on success.

**Description**:
This function performs the post-processing steps for Blazeface object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `od_fd_blazeface_pp_process_uint8`

**Purpose**:
Processes the Blazeface post-processing pipeline for uint8_t input data.

**Prototype**:
```c
int32_t od_fd_blazeface_pp_process_uint8(od_fd_blazeface_pp_in_t *pInput,
                                  od_pp_out_t *pOutput,
                                  od_fd_blazeface_pp_static_param_t *pInput_static_param);
```

**Parameters**:
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:
This function performs the post-processing steps for Blazeface object detection with uint8_t input data. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `od_fd_blazeface_pp_process_int8`

**Purpose**:
Processes the Blazeface post-processing pipeline for int8_t input data.

**Prototype**:
```c
int32_t od_fd_blazeface_pp_process_int8(od_fd_blazeface_pp_in_t *pInput,
                                  od_pp_out_t *pOutput,
                                  od_fd_blazeface_pp_static_param_t *pInput_static_param);
```

**Parameters**:
- **pInput**: Pointer to the post-processing input structure.
- **pOutput**: Pointer to the post-processing output structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:
- **AI_OD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:
This function performs the post-processing steps for Blazeface object detection with int8_t input data. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_OD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_OD_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

</details>

# Instance Segmentation Post-Processings
<details>

# YOLOV8 Instance Segmentation Post-Processing
<details>

## YOLOv8 Instance Segmentation Structures
---
### `iseg_yolov8_pp_in_centroid_t`

This structure is used for YOLOv8 Seg post-processing input with the raw detections and the raw masks.

Parameters:

- **void \*pRaw_detections**: Pointer to raw detection data in int8_t format.
- **void \*pRaw_masks**: Pointer to raw detection data in int8_t format.

---
### `iseg_yolov8_pp_static_param_t`

This structure holds the static parameters required for YOLOv8 Seg post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_total_boxes**: Total number of boxes predicted by the model. To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **int32_t nb_detect**: Number of detections after post-processing.
- **int32_t nb_masks**: number of masks. To extract fom the model output shape.
- **int32_t size_masks**: width of the masks. To extract fom the model output shape.
- **int8_t raw_output_zero_point**: Zero point for the quantized detections raw output values.
- **float32_t raw_output_scale**: Scale factor for the raw detections output values.
- **int8_t mask_raw_output_zero_point**: Zero point for the quantized masks raw output values.
- **float32_t mask_raw_output_scale**: Scale factor for the raw masks output values.
- **float32_t \*pMask**: pointer to buffer holding temporary float32_t mask values (AI_YOLOV8_SEG_PP_MASK_NB elements)
- **iseg_yolov8_pp_scratchBuffer_s8_t \*pTmpBuff**: scratch buffer holding structures with int8_t elements (AI_YOLOV8_SEG_PP_TOTAL_BOXES elements)


#### `iseg_yolov8_pp_scratchBuffer_s8_t`
This structure holds temporary data required for YOLOv8 instance segmentation post-processing with int8_t inputs.

Parameters:
- **int8_t x_center**: temporary data for iseg yolov8 post-processing
- **int8_t y_center**: temporary data for iseg yolov8 post-processing
- **int8_t width**: temporary data for iseg yolov8 post-processing
- **int8_t height**: temporary data for iseg yolov8 post-processing
- **int8_t conf**: temporary data for iseg yolov8 post-processing
- **uint8_t class_index**: temporary data for iseg yolov8 post-processing
- **int8_t \*pMask**: Pointer to int8_t temporary data mask buffer (AI_YOLOV8_SEG_PP_MASK_NB)

---
## YOLOv8 Seg Routines
---

### Pointer initialization

Pointers in iseg_pp_outBuffer_t, iseg_pp_scratchBuffer_s8_t and iseg_yolov8_pp_static_param_t needs to be initialized.

```c
float32_t _tmp_buf_mask[AI_YOLOV8_SEG_PP_MASK_NB];

int8_t _out_buf_mask_s8[AI_YOLOV8_SEG_PP_MASK_NB * AI_YOLOV8_SEG_PP_TOTAL_BOXES]; // output from model
iseg_pp_scratchBuffer_s8_t scratch_detections[AI_YOLOV8_SEG_PP_TOTAL_BOXES]; // internal use keep data in int8_t

uint8_t _iseg_mask[AI_YOLOV8_SEG_PP_MASK_SIZE*AI_YOLOV8_SEG_PP_MASK_SIZE * AI_YOLOV8_SEG_PP_MAX_BOXES_LIMIT]; // mask MASK_SIZE*MASK_SIZE for each output detection
iseg_pp_outBuffer_t out_detections[AI_YOLOV8_SEG_PP_MAX_BOXES_LIMIT];

```
```c
 for (size_t i = 0; i < AI_YOLOV8_SEG_PP_TOTAL_BOXES; i++) {
    scratch_detections[i].pMask = &_out_buf_mask[i*AI_YOLOV8_SEG_PP_MASK_NB];
 }
 for (size_t i = 0; i < AI_YOLOV8_SEG_PP_MAX_BOXES_LIMIT; i++) {
    out_detections[i].pMask = &_iseg_mask[i * AI_YOLOV8_SEG_PP_MASK_SIZE*AI_YOLOV8_SEG_PP_MASK_SIZE];
 }

  iseg_yolov8_pp_static_param_t y8_input_param;
  y8_input_param.pMask = _tmp_buf_mask;
  y8_input_param.pTmpBuff = scratch_detections;

``` 
### `iseg_yolov8_pp_reset`

**Purpose**:  
Resets the static parameters for YOLOv8 seg post-processing.

**Prototype**:  
```c
int32_t iseg_yolov8_pp_reset(iseg_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_ISEG_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the YOLOv8 seg post-processing by setting the number of detected objects to zero.

---

### `iseg_yolov8_pp_process`

**Purpose**:  
Processes the YOLOv8 iseg post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t iseg_yolov8_pp_process_int8(iseg_yolov8_pp_in_centroid_t *pInput,
                                    iseg_pp_out_t *pOutput,
                                    iseg_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- AI_ISEG_POSTPROCESS_ERROR_NO on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv8 seg object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_ISEG_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_ISEG_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

</details>

# Semantic segmentation Post-Processings

<details>

# Deeplabv3 Semantic segmentation
<details>

## Deeplabv3 Semantic segmentation Structures
---
### `sseg_deeplabv3_pp_in_t`

This structure is used for Deeplabv3 pose post-processing input where the raw detections are in float32/uint8/int8_t format.

Parameters:

- **void \*pRaw_detections**: Pointer to raw detection data in float32_t or uint8_t format.

---
### `sseg_deeplabv3_pp_static_param_t`

This structure holds the static parameters required for Deeplabv3 pose post-processing.

Parameters:

- **uint32_t width**:  The width of the model output. To extract fom the model output shape.
- **uint32_t height**:  The height of the model output. To extract fom the model output shape.
- **uint32_t nb_classes**: classes number of the model output. To extract fom the model output shape.

---
## Deeplabv3 Semantic segmentation Routines
---
### `sseg_deeplabv3_pp_reset`

**Purpose**:  
Resets the static parameters for Deeplabv3 semantic segmentation post-processing.

**Prototype**:  
```c
int32_t sseg_deeplabv3_pp_reset(sseg_deeplabv3_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **sseg_deeplabv3_pp_static_param_t**: Pointer to the static parameters structure.

**Returns**:  
- **AI_SSEG_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the Deeplabv3 but is currently no required (present to keep same API structure).

---

### `sseg_deeplabv3_pp_process`

**Purpose**:  
Processes the Deeplabv3 semantic segmentation post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t sseg_deeplabv3_pp_process(sseg_deeplabv3_pp_in_t *pInput,
                                  sseg_pp_out_t *pOutput,
                                  sseg_deeplabv3_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_SSEG_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for Deeplabv3 single semantic segmentation. It retrieves the maximum probability location for each keypoint and return its position and probability.

---

### `sseg_deeplabv3_pp_process_int8`

**Purpose**:  
Processes the Deeplabv3 semantic segmentation post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t sseg_deeplabv3_pp_process_int8(sseg_deeplabv3_pp_in_t *pInput,
                                       sseg_pp_out_t *pOutput,
                                       sseg_deeplabv3_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_SSEG_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for Deeplabv3 single semantic segmentation. It retrieves the maximum probability location for each keypoint and return its position and probability.

---

### `sseg_deeplabv3_pp_process_uint8`

**Purpose**:  
Processes the Deeplabv3 semantic segmentation post-processing pipeline for uint8_t input data.

**Prototype**:  
```c
int32_t sseg_deeplabv3_pp_process_uint8(sseg_deeplabv3_pp_in_t *pInput,
                                        sseg_pp_out_t *pOutput,
                                        sseg_deeplabv3_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- AI_SEG_POSTPROCESS_ERROR_NO on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for Deeplabv3 single semantic segmentation. It retrieves the maximum probability location for each keypoint and return its position and probability.

---

### Error Codes

- **AI_SSEG_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_SSEG_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

</details>


# Multi-Pose Post-Processings

<details>

# YOLOV8 Multi-Pose Post-Processing
<details>

## YOLOv8 Pose Structures
---
### `mpe_yolov8_pp_in_centroid_t`

This structure is used for YOLOv8 pose post-processing input where the raw detections are in float32_t/int8_t format.

Parameters:

- **void \*pRaw_detections**: Pointer to raw detection data in float32_t/int8_t format.


---
### `mpe_yolov8_pp_static_param_t`

This structure holds the static parameters required for YOLOv8 pose post-processing.

Parameters:

- **int32_t nb_classes**: Number of classes in the detection model. To extract fom the model output shape.
- **int32_t nb_total_boxes**: Total number of boxes predicted by the model. To extract fom the model output shape.
- **int32_t max_boxes_limit**: Maximum number of boxes per class to be considered after post-processing.
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS).A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **float32_t raw_output_scale**: Scale factor for raw output values (**int8** input data).
- **int8_t raw_output_zero_point**: Zero point for quantized raw output values(**int8** input data).
- **int32_t nb_detect**: Number of detections after post-processing.
- **mpe_pp_scratchBuffer_s8_t \*pScratchBuffer**: pointer to scratch buffer containing structure with int8_t elements (AI_MPE_YOLOV8_PP_TOTAL_BOXES, **int8** input data).
---
## YOLOv8 Pose Routines

---
### Pointers initialization 
```c
mpe_pp_outBuffer_t out_detections[AI_MPE_YOLOV8_PP_TOTAL_BOXES];
mpe_pp_keyPoints_t out_keyPoints[AI_MPE_YOLOV8_PP_TOTAL_BOXES*AI_MPE_YOLOV8_PP_KEYPOINTS_NB];
```
```c
 for (int i = 0; i < AI_MPE_YOLOV8_PP_TOTAL_BOXES; i++) {
    out_detections[i].pKeyPoints = &out_keyPoints[i*AI_MPE_YOLOV8_PP_KEYPOINTS_NB];
 }

``` 
When int8_t post-processing is used, scratch buffer have to be initialiazed:
```c
mpe_pp_keyPoints_s8_t scratchBuffer_keyPoints[AI_MPE_YOLOV8_PP_TOTAL_BOXES*AI_MPE_YOLOV8_PP_KEYPOINTS_NB];
mpe_pp_scratchBuffer_s8_t scratchBuffer_detections[AI_MPE_YOLOV8_PP_TOTAL_BOXES];
mpe_yolov8_pp_static_param_t y8_input_param;
```
```c
  for (int i = 0; i < AI_MPE_YOLOV8_PP_TOTAL_BOXES; i++) {
      scratchBuffer_detections[i].pKeyPoints = &scratchBuffer_keyPoints[i*AI_MPE_YOLOV8_PP_KEYPOINTS_NB];
  }
  y8_input_param.pScratchBuffer = scratchBuffer_detections;
```


---
### `mpe_yolov8_pp_reset`

**Purpose**:  
Resets the static parameters for YOLOv8 pose post-processing.

**Prototype**:  
```c
int32_t mpe_yolov8_pp_reset(mpe_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_MPE_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the YOLOv8 pose post-processing by setting the number of detected objects to zero.

---

### `mpe_yolov8_pp_process`

**Purpose**:  
Processes the YOLOv8 pose post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t mpe_yolov8_pp_process(mpe_yolov8_pp_in_centroid_t *pInput,
                              mpe_pp_out_t *pOutput,
                              mpe_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_MPE_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv8 pose object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### `mpe_yolov8_pp_process_int8`

**Purpose**:  
Processes the YOLOv8 pose post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t mpe_yolov8_pp_process_int8(mpe_yolov8_pp_in_centroid_t *pInput,
                                   mpe_pp_out_t *pOutput,
                                   mpe_yolov8_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_MPE_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for YOLOv8 pose object detection. It first retrieves the neural network boxes, then applies Non-Maximum Suppression (NMS), and finally performs score re-filtering.

---

### Error Codes

- **AI_MPE_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_MPE_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

</details>


# Single-Pose Post-Processings

<details>

# MoveNet Single-Pose Post-Processing
<details>

## MoveNet Single Pose Structures
---
### `spe_movenet_pp_in_t`

This structure is used for MoveNet pose post-processing input where the raw detections are in float32_t/int8_t format.

Parameters:

- **void \*pRaw_detections**: Pointer to raw detection data in float32_t/int8_t format.


---
### `spe_movenet_pp_static_param_t`

This structure holds the static parameters required for MoveNet pose post-processing.

Parameters:

- **uint32_t heatmap_width**:  The width of the model output. To extract fom the model output shape.
- **uint32_t heatmap_height**:  The height of the model output. To extract fom the model output shape.
- **uint32_t nb_keypoints**: Keypoints number of the model output. To extract fom the model output shape.
- **float32_t raw_scale**: Scale factor for raw output values (**int8** input data).
- **int8_t raw_zero_point**: Zero point for quantized raw output values(**int8** input data).

---
## MoveNet Single Pose Routines
---
### `spe_movenet_pp_reset`

**Purpose**:  
Resets the static parameters for MoveNet pose post-processing.

**Prototype**:  
```c
int32_t spe_movenet_pp_reset(spe_movenet_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **spe_movenet_pp_static_param_t**: Pointer to the static parameters structure.

**Returns**:  
- **AI_SPE_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the MoveNet but is currently no required (present to keep same API structure).

---

### `spe_movenet_pp_process`

**Purpose**:  
Processes the MoveNet pose post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t spe_movenet_pp_process(spe_movenet_pp_in_t *pInput,
                               spe_pp_out_t     *pOutput,
                               spe_movenet_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_SPE_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for MoveNet single pose object detection. It retrieves the maximum probability location for each keypoint and return its position and probability.

---

### `spe_movenet_pp_process_int8`

**Purpose**:  
Processes the MoveNet pose post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t spe_movenet_pp_process(spe_movenet_pp_in_t *pInput,
                               spe_pp_out_t     *pOutput,
                               spe_movenet_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- **AI_SPE_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for MoveNet single pose object detection. It retrieves the maximum probability location for each keypoint and return its position and probability.

---

### Error Codes

- **AI_SPE_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_SPE_POSTPROCESS_ERROR**: Indicates an error occured during execution of the function.

---

</details>

</details>


# Palm Detection Post-Processings
<details>

# CNN_pd Palm Detection Post-Processing
<details>

##  CNN_pd palm detection Structures
---
### `pd_model_pp_in_t`

This structure holds the input parameters required for CNN_pd palm detection post-processing.

Parameters:

- **float32_t \*pProbs**:  pointer to probabilites buffer (output from model).
- **float32_t \*pBoxes**:  pointer to boxes buffer (output from model).

---
### `pd_model_pp_static_param_t`

This structure holds the static parameters required for CNN_pd palm detection post-processing.

Parameters:

- **uint32_t width**: The width of the model input image. To extract fom the model input shape.
- **uint32_t height**: The height of the model input image. To extract fom the model input shape.
- **uint32_t nb_keypoints**: The number of keypoints. To extract fom the model output shape ( dim is nb_keypoints + 4 (x/y,w/h))
- **float32_t conf_threshold**: Confidence threshold for filtering detections. High confidence helps filtering out low-confidence detections (False positives), However, it is essential to balance the threshold value to ensure that you do not miss too many true positives.
- **float32_t iou_threshold**: Intersection over Union (IoU) threshold for Non-Maximum Suppression (NMS). A high IoU threshold means that more overlapping will be allowed between boxes, while a lower threshold will allow less boxes to be retained.
- **uint32_t nb_total_boxes**: The total number of boxes at model output. To extract fom the model output shape.
- **uint32_t max_boxes_limit**: The maximum boxes at post-processing output
- **float32_t \*pAnchors**: A pointer to an array of anchor box dimensions. Each anchor box is defined by its width and height. The array should have a length of 2 x nb_anchors, where each pair of values represents the width and height of an anchor box.
- **float32_t boxe_scale**: Scale factor for raw boxes model ouput values (**int8** input data).
- **float32_t proba_scale**: Scale factor for raw confidences model ouput  values (**int8** input data).
- **int8_t boxe_zp**: Zero point for quantized raw boxes model ouput  values(**int8** input data).
- **int8_t proba_zp**: Zero point for quantized raw confidences model ouput  values(**int8** input data).

---
## CNN_pd Semantic segmentation Routines
---
### `pd_model_pp_reset`

**Purpose**:  
Resets the static parameters for CNN_pd palm detection post-processing.

**Prototype**:  
```c
int32_t pd_model_pp_reset(pd_model_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pd_model_pp_static_param_t**: Pointer to the static parameters structure.

**Returns**:  
- **AI_PD_POSTPROCESS_ERROR_NO** on success.

**Description**:  
This function initializes the static parameters for the CNN_pd but is currently no required (present to keep same API structure).

---

### `pd_model_pp_process`

**Purpose**:  
Processes the CNN_pd pose post-processing pipeline for float32_t input data.

**Prototype**:  
```c
int32_t pd_model_pp_process(pd_model_pp_in_t *pInput,
                            pd_pp_out_t *pOutput,
                            pd_model_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- *AI_PD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for CNN_pd palm detection. It retrieves the maximum probability location for each box and return its position, size, probability and keypoints

---

### `pd_model_pp_process_int8`

**Purpose**:  
Processes the CNN_pd pose post-processing pipeline for int8_t input data.

**Prototype**:  
```c
int32_t pd_model_pp_process_int8(pd_model_pp_in_t *pInput,
                                 pd_pp_out_t *pOutput,
                                 pd_model_pp_static_param_t *pInput_static_param);
```

**Parameters**:  
- **pInput**: Pointer to the input post-processing structure.
- **pOutput**: Pointer to the output post-processing structure.
- **pInput_static_param**: Pointer to the static parameters structure.

**Returns**:  
- *AI_PD_POSTPROCESS_ERROR_NO** on success, or an error code on failure.

**Description**:  
This function performs the post-processing steps for CNN_pd palm detection. It retrieves the maximum probability location for each box and return its position, size, probability and keypoints

---

### Error Codes

- **AI_PD_POSTPROCESS_ERROR_NO**: Indicates successful execution of the function.
- **AI_PD_POSTPROCESS_ERROR**: Indicates error during execution of the function.

---

</details>

</details>
