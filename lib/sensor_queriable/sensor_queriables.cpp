
#include "sensor_queriables.hpp"

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
void log_req(z_loaned_query_t *query)
{
    auto payload = z_query_payload(query);
    auto payload_led = z_bytes_len(payload);
    auto it = z_bytes_get_slice_iterator(payload);
    z_view_slice_t v;
    while (z_bytes_slice_iterator_next(&it, &v))
    {
        const uint8_t *slice_data = z_slice_data(z_view_slice_loan(&v));
        size_t slice_len = z_slice_len(z_view_slice_loan(&v));
        Serial.printf("Slice data: %u, length: %zu\n", *slice_data, slice_len);
    }

    /*
    Serial.println("Now encoding");
    
    auto encoding = z_query_encoding(query);
    Serial.printf("Encoding schema: '%u', value: ''\n", encoding->id, z_string_data(&encoding->schema));

    z_view_string_t parameter_str;
    z_query_parameters(query, &parameter_str);
    Serial.printf("Parameters: '%s'\n",
                  z_string_data(z_view_string_loan(&parameter_str)));
    auto attachment = z_query_attachment(query);
    if (attachment)
    {
        z_owned_string_t attachment_str;
        z_bytes_to_string(attachment, &attachment_str);
        Serial.printf("Attachment: '%s'\n", z_string_data(z_string_loan(&attachment_str)));
        z_string_drop(z_string_move(&attachment_str));
    }
    else
    {
        Serial.println("No attachment");
    }
*/
    /*
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_query_keyexpr(query), &keystr);
    size_t len = query->_val->_value.payload._slices._len;
    const z_loaned_string_t *loaned_str = z_view_string_loan(&keystr);
    LOG_INFOF("Received query: %s", z_string_data(loaned_str));
    const void *data = query->_val->_value.payload._slices._val;
    if (data == NULL)
    {
        LOG_INFO("No payload data");
        return;
    }
    // Print payload data
    Serial.print("Payload data: [");
    for (size_t i = 0; i < len; ++i)
    {
        uint8_t byte = ((uint8_t *)data)[i];
        Serial.print(byte);
        if (i < len - 1) {
            Serial.print(", ");
        }
    }
    Serial.println("]");
    */
    // LOG_INFOF("\tPayload data: [%s]", payload_str.c_str());
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

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
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
        Serial.println(payload[i]);
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

uint8_t * query_payload_to_bytes(const z_loaned_query_t *query, size_t *len)
{
    const z_loaned_bytes_t *payload = z_query_payload(query);
    auto payload_len = z_bytes_len(payload);
     uint8_t *data_out = (uint8_t *)malloc(payload_len);
    auto it = z_bytes_get_slice_iterator(payload);
    z_view_slice_t v;
    size_t pos = 0;
    while (z_bytes_slice_iterator_next(&it, &v)) {
        const uint8_t *slice_data = z_slice_data(z_view_slice_loan(&v));
        size_t slice_len = z_slice_len(z_view_slice_loan(&v));
        memcpy(data_out + pos, slice_data, slice_len);
        pos += slice_len;
    }
    if (len) {
        *len = payload_len;
    }
    return data_out;
}

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
    size_t buf_size = 0;
    auto buf = query_payload_to_bytes(query, &buf_size);
    auto req = ByteMsg::deserialize(buf, buf_size);
    free(buf);

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
    std::shared_ptr<bool> has_moved = std::make_shared<bool>(false);
    std::shared_ptr<bool> dxovf = std::make_shared<bool>(false);
    std::shared_ptr<bool> dyovf = std::make_shared<bool>(false);
    sensor->readMotionStatusRegister(has_moved, dxovf, dyovf);

    if (!*has_moved)
    {
        return;
    }

    int16_t delta_x = sensor->readDeltaX();
    int16_t delta_y = sensor->readDeltaY();

    if (dxovf)
    {
        long_time_delta_x += 0xFFFF + delta_x;
    }

    if (dyovf)
    {
        long_time_delta_y += 0xFFFF + delta_y;
    }

    auto msg = DeltaMsg(delta_x, delta_y, *dxovf, *dyovf);
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
        LOG_DEBUGF("Published data: %d, %d to %s", delta_x, delta_y, keyexpr_publisher.c_str());
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
