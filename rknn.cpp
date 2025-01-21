/*
 * implementation of RKNN class functions
*/

#include "rknn.h"

// RKNN::RKNN() {}

int RKNN::load_model(unsigned char *buffer, size_t size)
{
    if ( buffer == nullptr && size <= 0  ) {
        return -1;
    }
    model_data = buffer;
    model_size = size;
    return 0;
}

int RKNN::load_model(std::string path)
{
    std::fstream file(path, std::ios::in | std::ios::binary );
    if (file.is_open()){
        size_t size =0;
        /*unsigned*/ char *data;
        file.seekg(0,std::ios::end);
        size = file.tellg();

        data = (/*unsigned*/ char *)malloc(size);

        file.seekg(0,std::ios::beg);

        file.read(data,size);

        return load_model((unsigned char *)data,size);

    }
    else {
        return -1;
    }
}

int RKNN::init_model()
{

    int ret;
    rknn_context ctx;

    /* Create the neural network */
    ret = rknn_init(&ctx, model_data, model_size, 0, NULL);
    if (ret < 0)
    {
        qDebug() <<"rknn_init error ret="<< ret;
        return -1;
    }

    if (model_data)
    {
        free(model_data);
    }

    rknn_sdk_version version;
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if (ret < 0)
    {
        qDebug() <<"rknn_query RKNN_QUERY_SDK_VERSION error ret=" << ret;
        return -1;
    }
    qDebug() <<"sdk version: " << version.api_version << "driver version: " << version.drv_version;

    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &app_ctx.io_num, sizeof(rknn_input_output_num));
    if (ret < 0)
    {
        qDebug() <<"rknn_query RKNN_QUERY_IN_OUT_NUM error ret="<< ret;
        return -1;
    }
    qDebug() << "model input num: "<< app_ctx.io_num.n_input <<", output num: "<< app_ctx.io_num.n_output;

    rknn_tensor_attr *input_attrs = (rknn_tensor_attr *)malloc(app_ctx.io_num.n_input * sizeof(rknn_tensor_attr));
    memset(input_attrs, 0, app_ctx.io_num.n_input * sizeof(rknn_tensor_attr));
    for (int i = 0; i < app_ctx.io_num.n_input; i++)
    {
        input_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret < 0)
        {
            qDebug() << "rknn_query RKNN_QUERY_INPUT_ATTR error ret="<< ret;
            return -1;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }

    rknn_tensor_attr *output_attrs = (rknn_tensor_attr *)malloc(app_ctx.io_num.n_output * sizeof(rknn_tensor_attr));
    memset(output_attrs, 0, app_ctx.io_num.n_output * sizeof(rknn_tensor_attr));
    for (int i = 0; i < app_ctx.io_num.n_output; i++)
    {
        output_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret < 0)
        {
            qDebug() << "rknn_query RKNN_QUERY_OUTPUT_ATTR error ret=%d\n"<< ret;
            return -1;
        }
        dump_tensor_attr(&(output_attrs[i]));
    }

    app_ctx.input_attrs = input_attrs;
    app_ctx.output_attrs = output_attrs;
    app_ctx.rknn_ctx = ctx;

    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
    {
        qDebug() << "model is NCHW input fmt";
        app_ctx.model_channel = input_attrs[0].dims[1];
        app_ctx.model_height = input_attrs[0].dims[2];
        app_ctx.model_width = input_attrs[0].dims[3];
    }
    else
    {
        qDebug() << "model is NHWC input fmt";
        app_ctx.model_height = input_attrs[0].dims[1];
        app_ctx.model_width = input_attrs[0].dims[2];
        app_ctx.model_channel = input_attrs[0].dims[3];
    }
    qDebug() <<  "model input height= " <<  app_ctx.model_height<<"width= "<<   app_ctx.model_width <<"channel= " << app_ctx.model_channel;

    inited = true;

    return 0;

}

bool RKNN::is_inited()
{
    return inited;
}

int RKNN::release_model()
{
    if (app_ctx.rknn_ctx != 0)
    {
        rknn_destroy(app_ctx.rknn_ctx);
    }
    free(app_ctx.input_attrs);
    free(app_ctx.output_attrs);
    return 0;
}

int RKNN::inference(void * buffer,std::vector<float> *results)
{
    int ret = 0;

    // rknn_context ctx = app_ctx.rknn_ctx;
    // int model_width = app_ctx.model_width;
    // int model_height = app_ctx.model_height;
    // int model_channel = app_ctx.model_channel;

    if (buffer == nullptr) {
        ret = -1;
        return ret;
    }

    rknn_input inputs[app_ctx.io_num.n_input];
    rknn_output outputs[app_ctx.io_num.n_output];

    memset(inputs  ,0 ,sizeof(inputs ) );
    memset(outputs ,0 ,sizeof(outputs) );

    //  fix-me:
    //  input type and size are setted manually
    for (int i = 0; i < app_ctx.io_num.n_input; i++) {
        inputs[i].index = app_ctx.input_attrs[i].index;
        inputs[i].size  = app_ctx.input_attrs[i].n_elems  ;//app_ctx.model_channel * app_ctx.model_height * app_ctx.model_width;
        inputs[i].buf   = buffer;
        inputs[i].pass_through = 0;//app_ctx.input_attrs[i].pass_through;

        inputs[i].type = RKNN_TENSOR_UINT8;// app_ctx.input_attrs[i].type;
        inputs[i].fmt = app_ctx.input_attrs[i].fmt;
    }


    for (int i = 0; i < app_ctx.io_num.n_output; i++) {
        outputs[i].index      = app_ctx.output_attrs[i].index;
        outputs[i].want_float = 1;//app_ctx.output_attrs[i].type != RKNN_TENSOR_FLOAT16 || app_ctx.output_attrs->type != RKNN_TENSOR_FLOAT32 ;
    }


    ret = rknn_inputs_set(app_ctx.rknn_ctx, app_ctx.io_num.n_input, inputs);

    if (ret < 0)
    {
        qDebug() <<"rknn_inputs_set error ret="<< ret;
        return ret;
    }



    ret = rknn_run(app_ctx.rknn_ctx, NULL);
    if (ret < 0)
    {
        qDebug() <<"rknn_run error ret="<< ret;
        return ret;
    }


    ret = rknn_outputs_get(app_ctx.rknn_ctx, app_ctx.io_num.n_output, outputs, nullptr);
    if (ret < 0)
    {
        qDebug() <<"rknn_outputs_get error ret="<< ret;
        return ret;
    }

    results->resize(app_ctx.output_attrs->n_elems);

    for (int i = 0; i < app_ctx.io_num.n_output; i++) {
        for (int j = 0; j < app_ctx.output_attrs[i].n_elems; j++) {
            float *p = (float*)outputs[i].buf;
            // qDebug() << "outputs["<<i<<"].buf["<<j<<"]: "<< p[j];
            results->at(j) = p[j] ;
        }
    }

    ret = rknn_outputs_release(app_ctx.rknn_ctx, app_ctx.io_num.n_output, outputs);
    if (ret < 0)
    {
        qDebug() <<"rknn_outputs_release error ret="<< ret;
        return ret;
    }

    ret = 0;

    return ret;
}
