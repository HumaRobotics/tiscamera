
#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include <vector>
#include <string>
#include <memory>
#include <mutex>

#include "utils.h"
#include "logging.h"
#include "device_discovery.h"

#include "Error.h"

#if HAVE_ARAVIS
#include "aravis_utils.h"
#endif

#if HAVE_USB
#include "v4l2_utils.h"
#endif

using namespace tcam;


std::vector<DeviceInfo> DeviceIndex::getDeviceList () const
{
    return device_list;
}


DeviceIndex::DeviceIndex ()
    : continue_thread(false), wait_period(2000000)
{
    continue_thread = true;
    work_thread = std::thread(&DeviceIndex::run, this);
    device_list = std::vector<DeviceInfo>();
}


DeviceIndex::~DeviceIndex ()
{
    if (continue_thread)
    {
        continue_thread = false;
        work_thread.join();
    }
}


void DeviceIndex::updateDeviceList ()
{
    std::vector<DeviceInfo> tmp_dev_list;

#if HAVE_ARAVIS
    auto aravis_dev_list = get_aravis_device_list();
    tmp_dev_list.insert(tmp_dev_list.end(), aravis_dev_list.begin(), aravis_dev_list.end());
#endif

#if HAVE_USB
    auto v4l2_dev_list = get_v4l2_device_list();
    tmp_dev_list.insert(tmp_dev_list.end(), v4l2_dev_list.begin(), v4l2_dev_list.end());
#endif

    std::mutex mtx;

    mtx.lock();

    device_list.clear();

    device_list.insert(device_list.end(), tmp_dev_list.begin(), tmp_dev_list.end());

    mtx.unlock();
}


void DeviceIndex::run ()
{
    while (continue_thread)
    {
        updateDeviceList();
        usleep(wait_period);
    }
}


std::shared_ptr<DeviceIndex> tcam::getDeviceIndex ()
{
    return std::make_shared<DeviceIndex>();
}