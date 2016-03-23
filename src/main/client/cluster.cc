/*******************************************************************************
 * Copyright 2016 Aerospike, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

extern "C" {
    #include <aerospike/aerospike.h>
    #include <aerospike/as_async_proto.h>
}

#include <node.h>

#include "client.h"
#include "conversions.h"
#include "log.h"

using namespace v8;

/**
 * Connect to an Aerospike Cluster
 */
NAN_METHOD(AerospikeClient::Connect)
{
    Nan::HandleScope scope;
    AerospikeClient * client = ObjectWrap::Unwrap<AerospikeClient>(info.This());

    Local<Function> callback;
    if (info.Length() > 0 && info[0]->IsFunction()) {
        callback = Local<Function>::Cast(info[0]);
    } else {
        as_v8_error(client->log, "Callback function required");
        return Nan::ThrowError("Callback function required");
    }

    as_error err;
    aerospike_connect(client->as, &err);
    if (err.code != AEROSPIKE_OK) {
        as_v8_error(client->log, "Connecting to Cluster Failed: %s", err.message);
        client->as->cluster = NULL;
    } else {
        as_v8_debug(client->log, "Connecting to Cluster: Success");
    }

    Local<Value> argv[2];
    argv[0] = error_to_jsobject(&err, client->log);
    argv[1] = (info.Holder());
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, 2, argv);
}

/**
 *  Close the connections to the Aeropsike cluster.
 */
NAN_METHOD(AerospikeClient::Close)
{
	Nan::HandleScope scope;
	AerospikeClient * client = ObjectWrap::Unwrap<AerospikeClient>(info.This());

	as_v8_debug(client->log, "Closing the connection to aerospike cluster");
	as_error err;
	aerospike_close(client->as, &err);
	aerospike_destroy(client->as);
	free(client->as);
	free(client->log);
}

/**
 * Is cluster connected to any server nodes.
 */
NAN_METHOD(AerospikeClient::IsConnected)
{
    Nan::HandleScope scope;
    AerospikeClient * client = ObjectWrap::Unwrap<AerospikeClient>(info.This());

    bool connected = aerospike_cluster_is_connected(client->as);

	info.GetReturnValue().Set(Nan::New(connected));
}

/**
 * Are there any pending async commands.
 */
NAN_METHOD(AerospikeClient::HasPendingAsyncCommands)
{
	Nan::HandleScope scope;
	AerospikeClient * client = ObjectWrap::Unwrap<AerospikeClient>(info.This());

	bool pending = as_async_get_pending(client->as->cluster) > 0;

	info.GetReturnValue().Set(Nan::New(pending));
}
