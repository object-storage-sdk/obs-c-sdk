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
void truncate_object(const obs_options *options, char *key, uint64_t object_length,
	obs_response_handler *handler, void *callback_data)
{
	request_params params;
	obs_use_api use_api = OBS_USE_API_S3;
	set_use_api_switch(options, &use_api);
	string_buffer(queryParams, QUERY_STRING_LEN);
	string_buffer_initialize(queryParams);
	int amp = 0;
	char strToAppend[128] = { 0 };

	COMMLOG(OBS_LOGINFO, "Enter truncate_object successfully !");

	if (!options->bucket_options.bucket_name) {
		COMMLOG(OBS_LOGERROR, "bucket_name is NULL!");
		(void)(*(handler->complete_callback))(OBS_STATUS_InvalidBucketName, 0, callback_data);
		return;
	}

	int ret = snprintf_s(strToAppend, sizeof(strToAppend), _TRUNCATE, "%lu", object_length);
	CheckAndLogNeg(ret, "snprintf_s", __FUNCTION__, __LINE__);
	safe_append("length", strToAppend, sizeof(strToAppend), handler->complete_callback);

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
	params.subResource = "truncate";
	params.key = key;
	params.properties_callback = handler->properties_callback;
	params.complete_callback = handler->complete_callback;
	params.callback_data = callback_data;
	params.isCheckCA = is_check_ca(options);
	params.storageClassFormat = no_need_storage_class;
	params.use_api = use_api;

	request_perform(&params);
	COMMLOG(OBS_LOGINFO, "Leave truncate_object successfully !");
}