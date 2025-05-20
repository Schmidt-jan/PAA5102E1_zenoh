#include "sensor_queriables.hpp"

#if LOG_LEVEL <= LOG_LEVEL_INFO
void log_req(z_loaned_query_t *query)
{
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_query_keyexpr(query), &keystr);
    const z_loaned_string_t *loaned_str = z_view_string_loan(&keystr);
    LOG_INFOF("Received query: %s", z_string_data(loaned_str));
    size_t len = query->_val->_value.payload._slices._len;
    LOG_INFOF("\tPayload length: %zu", len);
    const void *data = query->_val->_value.payload._slices._val;
    std::string payload_str = "";
    for (size_t i = 0; i < len; ++i)
    {
        payload_str += std::to_string(reinterpret_cast<const uint8_t *>(data)[i]);
        if (i < len - 1)
            payload_str += ", ";
    }
    LOG_INFOF("\tPayload data: [%s]", payload_str.c_str());
}
#define LOG_REQ(query)  \
    do                  \
    {                   \
        log_req(query); \
    } while (0)
#else
#define LOG_REQ(query) \
    do                 \
    {                  \
    } while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
void log_reply(z_loaned_query_t *query, uint8_t *payload, size_t len)
{
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_query_keyexpr(query), &keystr);
    const z_loaned_string_t *loaned_str = z_view_string_loan(&keystr);
    LOG_INFOF("Reply to query: %s", z_string_data(loaned_str));
    LOG_INFOF("\tPayload length: %zu", len);
    std::string payload_str = "";
    for (size_t i = 0; i < len; ++i)
    {
        payload_str += std::to_string(payload[i]);
        if (i < len - 1)
            payload_str += ", ";
    }
    LOG_INFOF("\tPayload data: [%s]", payload_str.c_str());
}
#define LOG_REPLY(query, payload, len)  \
    do                                  \
    {                                   \
        log_reply(query, payload, len); \
    } while (0)
#else
#define LOG_REPLY(query, payload, len) \
    do                                 \
    {                                  \
    } while (0)
#endif

void send_query_reply(z_loaned_query_t *query, uint8_t *payload, size_t len)
{
    LOG_REPLY(query, payload, len);
    z_owned_bytes_t bytes;
    z_bytes_from_static_buf(&bytes, payload, len);
    z_query_reply(query, z_query_keyexpr(query), z_bytes_move(&bytes), NULL);
}

void q_reset(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    handler->sensor->reset();

    auto msg = EmptyMsg();
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);

    send_query_reply(query, buf, buffer_size);
}

void q_sleep(z_loaned_query_t *query, void *arg)
{

    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    handler->sensor->sleep();

    auto msg = EmptyMsg();
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_awake(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    handler->sensor->awake();

    auto msg = EmptyMsg();
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_isWriteProtected(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->isWriteProtected();

    auto msg = BoolMsg(res);
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_isSleeping(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->isSleeping();

    auto msg = BoolMsg(res);
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_isAwake(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->isAwake();

    auto msg = BoolMsg(res);
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_enableWriteProtection(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    handler->sensor->enableWriteProtection();

    auto msg = EmptyMsg();
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_disableWriteProtection(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    handler->sensor->disableWriteProtection();

    auto msg = EmptyMsg();
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_writeLaserDriveCurrent(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    const z_loaned_bytes_t *payload = z_query_payload(query);
    auto buf_size = payload->_slices._len;
    uint8_t buf[buf_size] = {};
    auto req = ByteMsg::deserialize(buf, buf_size);

    handler->sensor->writeLaserDriveCurrent(req->value);

    auto rep = EmptyMsg();
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
    delete req;
}

void q_readLaserDriveCurrent(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readLaserDriveCurrent();

    auto rep = ByteMsg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_readDeltaX(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readDeltaX();

    auto rep = UInt16Msg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_readDeltaY(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readDeltaY();

    auto rep = UInt16Msg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_readShutter(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readShutter();

    auto rep = ByteMsg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_readFrameAvg(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readFrameAvg();

    auto rep = ByteMsg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_readImageQuality(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readImageQuality();

    auto rep = UInt16Msg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_readResolutionX(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readResolutionX();

    auto rep = ByteMsg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_readResolutionY(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    auto res = handler->sensor->readResolutionY();

    auto rep = ByteMsg(res);
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
}

void q_writeResolutionX(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    const z_loaned_bytes_t *payload = z_query_payload(query);
    auto buf_size = payload->_slices._len;
    uint8_t buf[buf_size] = {};
    auto req = ByteMsg::deserialize(buf, buf_size);

    handler->sensor->writeResolutionX(req->value);

    auto rep = EmptyMsg();
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
    delete req;
}

void q_writeResolutionY(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    const z_loaned_bytes_t *payload = z_query_payload(query);
    auto buf_size = payload->_slices._len;
    uint8_t buf[buf_size] = {};
    auto req = ByteMsg::deserialize(buf, buf_size);

    handler->sensor->writeResolutionY(req->value);

    auto rep = EmptyMsg();
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
    delete req;
}

// --- Start / Stop / Set Frequency ---

void q_start(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    handler->sending_enabled = true;

    LOG_INFO("Start publishing delta data");

    auto msg = EmptyMsg();
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_stop(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    handler->sending_enabled = false;

    LOG_INFO("Stopped publishing delta data");

    auto msg = EmptyMsg();
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);
    send_query_reply(query, buf, buffer_size);
}

void q_setFrequency(z_loaned_query_t *query, void *arg)
{
    LOG_REQ(query);
    Z_PAA5102E1_Handler *handler = static_cast<Z_PAA5102E1_Handler *>(arg);
    const z_loaned_bytes_t *payload = z_query_payload(query);
    auto buf_size = payload->_slices._len;
    uint8_t buf[buf_size] = {};
    auto req = UInt16Msg::deserialize(buf, buf_size);

    handler->send_freq_hz = req->value;

    LOG_INFOF("Set publisher frequency to %d", handler->send_freq_hz);

    auto rep = EmptyMsg();
    auto buffer_size = rep.serialized_size();
    uint8_t rep_buf[buffer_size] = {};
    rep.serialize(rep_buf, buffer_size);
    send_query_reply(query, rep_buf, buffer_size);
    delete req;
}

///////////////////////////////////////////////////
// Sensordaten senden

void Z_PAA5102E1_Handler::send_sensor_data()
{
    bool dxovf = false;
    bool dyovf = false;
    bool has_moved = false;

    sensor->readMotionStatusRegister(&has_moved, &dxovf, &dyovf);

    int16_t delta_x = 0;
    int16_t delta_y = 0;
    if (has_moved)
    {
        delta_x = sensor->readDeltaX();
        delta_y = sensor->readDeltaY();
    }

    if (dxovf)
    {
        long_time_delta_x += 0xFFFF + delta_x;
    }

    if (dyovf)
    {
        long_time_delta_y += 0xFFFF + delta_y;
    }

    auto msg = DeltaMsg(delta_x, delta_y, dxovf, dyovf);
    auto buffer_size = msg.serialized_size();
    uint8_t buf[buffer_size] = {};
    msg.serialize(buf, buffer_size);

    z_owned_bytes_t bytes;
    z_bytes_from_static_buf(&bytes, buf, buffer_size);
    auto res = z_publisher_put(z_publisher_loan_mut(&pub), z_bytes_move(&bytes), NULL);
    if (res < 0)
    {
        LOG_ERROR("Failed to publish data");
    }
    else
    {
        LOG_DEBUGF("Published data: %d, %d\n", delta_x, delta_y);
    }
}

void Z_PAA5102E1_Handler::declare_queryable(const std::string &keyexpr_suf, void (*callback)(z_loaned_query_t *, void *), z_owned_queryable_t &queryable)
{
    const auto full_keyexpr = name + "/" + keyexpr_suf;
    z_owned_closure_query_t cb;
    z_closure_query(&cb, callback, NULL, this);
    z_view_keyexpr_t ke;
    z_view_keyexpr_from_str_unchecked(&ke, full_keyexpr.c_str());

    if (z_declare_queryable(session, &queryable, z_view_keyexpr_loan(&ke), z_closure_query_move(&cb), NULL) < 0)
    {
        LOG_ERRORF("Failed to declare queryable: %s", full_keyexpr.c_str());
    }
    else
    {
        LOG_INFOF("Declared queryable: %s", full_keyexpr.c_str());
    }
}

void Z_PAA5102E1_Handler::setup_queryables()
{
    declare_queryable("system/reset", q_reset, qable_reset);
    declare_queryable("system/sleep", q_sleep, qable_sleep);
    declare_queryable("system/awake", q_awake, qable_awake);
    declare_queryable("system/isWriteProtected", q_isWriteProtected, qable_isWriteProtected);
    declare_queryable("system/isSleeping", q_isSleeping, qable_isSleeping);
    declare_queryable("system/isAwake", q_isAwake, qable_isAwake);
    declare_queryable("system/enableWriteProtection", q_enableWriteProtection, qable_enableWriteProtection);
    declare_queryable("system/disableWriteProtection", q_disableWriteProtection, qable_disableWriteProtection);

    declare_queryable("read/LaserDriveCurrent", q_readLaserDriveCurrent, qable_readLaserDriveCurrent);
    declare_queryable("read/DeltaX", q_readDeltaX, qable_readDeltaX);
    declare_queryable("read/DeltaY", q_readDeltaY, qable_readDeltaY);
    declare_queryable("read/Shutter", q_readShutter, qable_readShutter);
    declare_queryable("read/FrameAvg", q_readFrameAvg, qable_readFrameAvg);
    declare_queryable("read/ImageQuality", q_readImageQuality, qable_readImageQuality);
    declare_queryable("read/ResolutionX", q_readResolutionX, qable_readResolutionX);
    declare_queryable("read/ResolutionY", q_readResolutionY, qable_readResolutionY);

    declare_queryable("write/LaserDriveCurrent", q_writeLaserDriveCurrent, qable_writeLaserDriveCurrent);
    declare_queryable("write/ResolutionX", q_writeResolutionX, qable_writeResolutionX);
    declare_queryable("write/ResolutionY", q_writeResolutionY, qable_writeResolutionY);

    declare_queryable("publisher/start", q_start, qable_start);
    declare_queryable("publisher/stop", q_stop, qable_stop);
    declare_queryable("publisher/setFrequency", q_setFrequency, qable_setFrequency);
}

void Z_PAA5102E1_Handler::loop()
{
    if (!sending_enabled)
    {
        return;
    }

    if (last_send == 0)
    {
        last_send = millis();
        send_sensor_data();
        return;
    }

    unsigned long now = millis();
    if (now - last_send >= 1000 / send_freq_hz)
    {
        send_sensor_data();
        last_send = now;
    }
}