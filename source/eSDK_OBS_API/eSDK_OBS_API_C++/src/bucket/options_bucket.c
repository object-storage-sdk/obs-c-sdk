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
#include <string.h>
#include <stdlib.h>
#include "eSDKOBS.h"
#include "request.h"
#include "simplexml.h"
#include "securec.h"
#include "util.h"
#include "bucket.h"
#include "request_util.h"
#include <openssl/md5.h> 

void obs_options_bucket(const obs_options *options, char* origin,
    char(*request_method)[OBS_COMMON_LEN_256], unsigned int method_number,
    char(*request_header)[OBS_COMMON_LEN_256], unsigned int header_number,
    obs_response_handler *handler, void *callback_data)
{
    COMMLOG(OBS_LOGINFO, "Enter %s successfully !", __FUNCTION__);

    obs_options_obj_or_bucket(options, 1, NULL, origin, request_method, method_number,
        request_header, header_number, handler, callback_data);

    COMMLOG(OBS_LOGINFO, "Leave %s successfully !", __FUNCTION__);

}