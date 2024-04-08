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
#include "eSDKOBS.h"
#include "request.h"
#include "securec.h"
#include "request_util.h"

// only posix bucke use
void modify_object(const obs_options *options, char *key, uint64_t content_length, uint64_t position,
	obs_put_properties *put_properties, server_side_encryption_params *encryption_params,
	obs_modify_object_handler *handler, void *callback_data)
{
	request_params params;
	obs_use_api use_api = OBS_USE_API_S3;
	set_use_api_switch(options, &use_api);
	string_buffer(queryParams, QUERY_STRING_LEN);
	string_buffer_initialize(queryParams);
	int amp = 0;
	char strToAppend[128] = { 0 };
	COMMLOG(OBS_LOGINFO, "Enter modify_object successfully !");
	if (!options->bucket_options.bucket_name) {
		COMMLOG(OBS_LOGERROR, "bucket_name is NULL!");
		(void)(*(handler->response_handler.complete_callback))(OBS_STATUS_InvalidBucketName, 0, callback_data);
		return;
	}
	int ret = snprintf_s(strToAppend, sizeof(strToAppend), _TRUNCATE, "%lu", position);
	CheckAndLogNeg(ret, "snprintf_s", __FUNCTION__, __LINE__);
	safe_append("position", strToAppend, sizeof(strToAppend), handler->response_handler.complete_callback);
	memset_s(&params, sizeof(request_params), 0, sizeof(request_params));
	errno_t err = EOK;
	err = memcpy_s(&params.bucketContext, sizeof(obs_bucket_context), &options->bucket_options,
		sizeof(obs_bucket_context));
	CheckAndLogNoneZero(err, "memcpy_s", __FUNCTION__, __LINE__);
	err = memcpy_s(&params.request_option, sizeof(obs_http_request_option), &options->request_options,
		sizeof(obs_http_request_option));
	CheckAndLogNoneZero(err, "memcpy_s", __FUNCTION__, __LINE__);
	params.temp_auth = options->temp_auth;
	params.httpRequestType = http_request_type_put;
	params.queryParams = queryParams[0] ? queryParams : 0;
	params.subResource = "modify";
	params.key = key;
	params.put_properties = put_properties;
	params.encryption_params = encryption_params;
	params.toObsCallback = handler->modify_object_data_callback;
	params.toObsCallbackTotalSize = content_length;
	params.properties_callback = handler->response_handler.properties_callback;
	params.complete_callback = handler->response_handler.complete_callback;
	params.callback_data = callback_data;
	params.isCheckCA = is_check_ca(options);
	params.storageClassFormat = storage_class;
	params.use_api = use_api;
	request_perform(&params);
	COMMLOG(OBS_LOGINFO, "Leave modify_object successfully !");
}
