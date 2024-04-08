/*********************************************************************************

* Licensed under the Apache License, Version 2.0 (the "License"); you may not use
* this file except in compliance with the License.  You may obtain a copy of the
* License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software distributed
* under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations under the License.
**********************************************************************************
*/
#include "bucket.h"
#include "request_util.h"
#include <openssl/md5.h> 

void get_bucket_metadata_with_corsconf(const obs_options *options, char *origin,
    char(*requestHeader)[OBS_COMMON_LEN_256], unsigned int number,
    obs_response_handler *handler)
{
    request_params params;
    unsigned int i = 0;
    obs_use_api use_api = OBS_USE_API_S3;
    set_use_api_switch(options, &use_api);
    COMMLOG(OBS_LOGINFO, "get bucket metadata with corsconf start!");

    obs_cors_conf cors_conf;

    memset_s(&cors_conf, sizeof(cors_conf), 0, sizeof(cors_conf));

    cors_conf.origin = origin;
    cors_conf.rhNumber = number;
    for (; i < number; i++)
    {
        cors_conf.requestHeader[i] = requestHeader[i];
    }
    memset_s(&params, sizeof(request_params), 0, sizeof(request_params));
    errno_t err = EOK;
    err = memcpy_s(&params.bucketContext, sizeof(obs_bucket_context), &options->bucket_options,
        sizeof(obs_bucket_context));
    CheckAndLogNoneZero(err, "memcpy_s", __FUNCTION__, __LINE__);
    err = memcpy_s(&params.request_option, sizeof(obs_http_request_option), &options->request_options,
        sizeof(obs_http_request_option));
    CheckAndLogNoneZero(err, "memcpy_s", __FUNCTION__, __LINE__);

    params.httpRequestType = http_request_type_head;
    params.properties_callback = handler->properties_callback;
    params.complete_callback = handler->complete_callback;
    params.isCheckCA = is_check_ca(options);
    params.storageClassFormat = no_need_storage_class;
    params.corsConf = &cors_conf;
    params.temp_auth = options->temp_auth;
    params.use_api = use_api;
    request_perform(&params);
    COMMLOG(OBS_LOGINFO, "get bucket metadata with corsconf finish!");
}