#ifndef SENSEKITCONTEXT_H
#define SENSEKITCONTEXT_H

#include <SenseKit.h>
#include <atomic>
#include "PluginService.h"
#include "StreamSet.h"

struct StreamServiceProxyBase;
struct PluginServiceProxyBase;

namespace sensekit {

    class SenseKitContext;

    using initialize_fn = void(*)(StreamServiceProxyBase*, PluginServiceProxyBase*);
    using terminate_fn = void(*)();
    using update_fn = void(*)();

    struct PluginFuncs
    {
        initialize_fn initialize{nullptr};
        terminate_fn terminate{nullptr};
        update_fn update{nullptr};

        bool isValid()
        {
            return initialize != nullptr &&
                   terminate != nullptr &&
                   update != nullptr;
        }
    };

    class SenseKitContext
    {
    public:
        SenseKitContext()
            : m_pluginService(*this) {}
        virtual ~SenseKitContext() {}

        sensekit_status_t initialize();
        sensekit_status_t terminate();

        sensekit_status_t open_streamset(const char* uri, sensekit_streamset_t*& streamset);
        sensekit_status_t close_streamset(sensekit_streamset_t*& streamset);

        sensekit_status_t open_stream(sensekit_streamset_t* streamset,
                                      sensekit_stream_type_t type,
                                      sensekit_stream_subtype_t subtype,
                                      sensekit_streamconnection_t*& streamConnection);

        sensekit_status_t close_stream(sensekit_streamconnection_t*& streamConnection);

        sensekit_status_t open_frame(sensekit_streamconnection_t* streamConnection,
                                     int timeout,
                                     sensekit_frame_ref_t*& frameRef);

        sensekit_status_t close_frame(sensekit_frame_ref_t*& frameRef);

        sensekit_status_t temp_update();

        StreamSet& get_rootSet() { return m_rootSet; }

        sensekit_status_t set_parameter(sensekit_streamconnection_t* streamConnection,
                                        sensekit_parameter_id parameterId,
                                        size_t byteLength,
                                        sensekit_parameter_data_t* data);

        sensekit_status_t get_parameter_size(sensekit_streamconnection_t* streamConnection,
                                             sensekit_parameter_id parameterId,
                                             size_t& byteLength);

        sensekit_status_t get_parameter_data(sensekit_streamconnection_t* streamConnection,
                                             sensekit_parameter_id parameterId,
                                             size_t byteLength,
                                             sensekit_parameter_data_t* data);

        StreamServiceProxyBase* get_streamServiceProxy() { return m_streamServiceProxy; }

    private:
        StreamSet m_rootSet;

        PluginService m_pluginService;
        PluginServiceProxyBase* m_pluginServiceProxy;
        StreamServiceProxyBase* m_streamServiceProxy;

        using PluginList = std::vector<PluginFuncs>;
        PluginList m_pluginList;
    };
}

#endif /* SENSEKITCONTEXT_H */
