/*
 * this class tries to be General RKNN api implementation for RKNN model loading and inferencing.
 * this file is only use for RKNN npu and has no use on other platforms
*/

#ifndef RKNN_H
#define RKNN_H


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include <QDebug>
#include <vector>

// rknn_api
#include <rknn_api.h>

// using namespace std;

class RKNN {

public:
    RKNN(){}

    typedef struct
    {
        rknn_context rknn_ctx;
        rknn_input_output_num io_num;
        rknn_tensor_attr *input_attrs;
        rknn_tensor_attr *output_attrs;
        int model_channel;
        int model_width;
        int model_height;
    } rknn_app_context_t;




    int  load_model(unsigned char *  buffer ,size_t size);
    int  load_model(std::string path);

    static void dump_tensor_attr(rknn_tensor_attr *attr)
    {
        qDebug () <<
            "index="<<attr->index <<
            "name=" << attr->name <<
            "n_dims="<< attr->n_dims <<
            "dims="<<
            attr->dims[0]<<
            attr->dims[1]<<
            attr->dims[2]<<
            attr->dims[3]<<
            "n_elems="<< attr->n_elems<<
            "size="<<attr->size<<
            "pass_through="<< attr->pass_through<<
            "fmt="<<get_format_string(attr->fmt)<<
            "type="<< get_type_string(attr->type)<<
            "qnt_type="<< get_qnt_type_string(attr->qnt_type)<<
            "zp="<<attr->zp<<
            "scale="<<attr->scale ;
    }


    int  init_model();
    bool is_inited();
    int  release_model();

    int inference(void * buffer,std::vector<float> *results);

private:
    bool inited = false;
    rknn_app_context_t  app_ctx;
    const std::string model_path= "";
    unsigned char * model_data = nullptr;
    size_t model_size =0;
};

#endif // RKNN_H
