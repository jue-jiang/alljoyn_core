/**
 * @file
 * Singleton for the AllJoyn Android Wi-Fi Direct (Wi-Fi P2P) connection manager
 */

/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <qcc/Debug.h>

#include "P2PConMan.h"
#include "P2PConManImpl.h"

#define QCC_MODULE "P2PCM"

namespace ajn {

P2PConMan::P2PConMan()
    : m_constructed(false), m_destroyed(false), m_refCount(0), m_pimpl(NULL)
{
    QCC_DbgPrintf(("P2PConMan::P2PConMan()"));
    m_constructed = true;
}

P2PConMan::~P2PConMan()
{
    QCC_DbgPrintf(("P2PConMan::~P2PConMan()"));

    m_destroyed = true;

    //
    // Unfortunately, at global static object destruction time, it is too late
    // to be calling into the private implementation which is going to be
    // indirectly talking to another helper object which is talking to the
    // AllJoyn DBus interface.  We have to ensure that this object goes away
    // while there is enough infrastructure left in the AllJoyn bus to acquire
    // locks, etc., that it may need.
    //
    // So, by the time we get here, there had better not be a private
    // implementation object left around, since we will most likely crash if we
    // try to delete it.
    //
    assert(m_pimpl == NULL && "P2PConMan::P2PConMan(): private implementation not deleted");
}

#define ASSERT_STATE(function) \
    { \
        assert(m_constructed && "P2PConMan::" # function "(): Singleton not constructed"); \
        assert(!m_destroyed && "P2PConMan::" # function "(): Singleton destroyed"); \
        assert(m_pimpl && "P2PConMan::" # function "(): Private impl is NULL"); \
    }

void P2PConMan::Acquire(BusAttachment* const bus, const qcc::String& guid)
{
    QCC_DbgPrintf(("P2PConMan::Acquire()"));

    //
    // If the entry gate has been closed, we do not allow an Acquire to actually
    // acquire a reference.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return;
    }

    //
    // The only way someone can get to us is if they call Instance() which will
    // cause the object to be constructed.
    //
    assert(m_constructed && "P2PConMan::Acquire(): Singleton not constructed");

    int refs = qcc::IncrementAndFetch(&m_refCount);
    if (refs == 1) {
        m_pimpl = new P2PConManImpl;

        ASSERT_STATE("Acquire");

        //
        // The first transport in gets to set the GUID.  There should be only
        // one GUID associated with a daemon process, so this should never
        // change.
        //
        Init(bus, guid);
        Start();
    }
}

void P2PConMan::Release()
{
    QCC_DbgPrintf(("P2PConMan::Release()"));

    //
    // If the entry gate has been closed, we do not allow a Release to actually
    // release a reference.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return;
    }

    ASSERT_STATE("Release");
    int refs = qcc::DecrementAndFetch(&m_refCount);
    if (refs == 0) {
        QCC_DbgPrintf(("P2PConMan::Release(): refs == 0"));

        //
        // The last transport to release its interest in the name service gets
        // pay the price for waiting for the service to exit.  Since we do a
        // Join(), this method is expected to be called out of a transport's
        // Join() so the price is expected.
        //
        Stop();
        Join();

        //
        // Unfortunately, at global static object destruction time, it is too
        // late to be calling into the private implementation which is going to
        // be indirectly talking to another helper object which is talking to
        // the AllJoyn DBus interface.  We have to ensure that this object goes
        // away while there is enough infrastructure left in the AllJoyn bus to
        // acquire locks, etc., that it may need.  That is here and now.
        //
        delete m_pimpl;
        m_pimpl = NULL;
    }
}

QStatus P2PConMan::Start()
{
    QCC_DbgPrintf(("P2PConMan::Start()"));

    //
    // If the entry gate has been closed, we do not allow a Start to actually
    // start anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Start");
    return m_pimpl->Start();
}

bool P2PConMan::Started()
{
    QCC_DbgPrintf(("P2PConMan::Started()"));

    //
    // If the entry gate has been closed, we do not allow a Started to actually
    // test anything.  We just return false.  The singleton is going away and so
    // we assume we are running __run_exit_handlers() so main() has returned.
    // We are definitely shutting down, and the process is going to exit, so
    // tricking callers who may be temporarily running is okay.
    //
    if (m_destroyed) {
        return false;
    }

    ASSERT_STATE("Started");
    return m_pimpl->Started();
}

QStatus P2PConMan::Stop()
{
    QCC_DbgPrintf(("P2PConMan::Stop()"));

    //
    // If the entry gate has been closed, we do not allow a Stop to actually
    // stop anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.  The destructor is going to handle
    // the actual Stop() and Join().
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Stop");
    return m_pimpl->Stop();
}

QStatus P2PConMan::Join()
{
    QCC_DbgPrintf(("P2PConMan::Join()"));

    //
    // If the entry gate has been closed, we do not allow a Join to actually
    // join anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.  The destructor is going to handle
    // the actual Stop() and Join().
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Join");
    return m_pimpl->Join();
}

QStatus P2PConMan::Init(BusAttachment* const bus, const qcc::String& guid)
{
    QCC_DbgPrintf(("P2PConMan::Init()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Init");
    return m_pimpl->Init(bus, guid);
}

void P2PConMan::SetCallback(Callback<void, LinkState, const qcc::String&>* cb)
{
    if (m_destroyed) {
        return;
    }

    ASSERT_STATE("SetCallback");
    m_pimpl->SetCallback(cb);
}

QStatus P2PConMan::CreateTemporaryNetwork(const qcc::String& device, int32_t intent)
{
    QCC_DbgPrintf(("P2PConMan::CreateTemporaryNetwork()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("CreateTemporaryNetwork");
    return m_pimpl->CreateTemporaryNetwork(device, intent);
}

QStatus P2PConMan::DestroyTemporaryNetwork(void)
{
    QCC_DbgPrintf(("P2PConMan::DestroyTemporaryNetwork()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("DestroyTemporaryNetwork");
    return m_pimpl->DestroyTemporaryNetwork();
}

bool P2PConMan::IsConnected(const qcc::String& device)
{
    QCC_DbgPrintf(("P2PConMan::IsConnected()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return false;
    }

    ASSERT_STATE("IsConnected");
    return m_pimpl->IsConnected(device);
}

bool P2PConMan::IsConnected(void)
{
    QCC_DbgPrintf(("P2PConMan::IsConnected()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return false;
    }

    ASSERT_STATE("IsConnected");
    return m_pimpl->IsConnected();
}

bool P2PConMan::IsConnectedSTA(void)
{
    QCC_DbgPrintf(("P2PConMan::IsConnectedSTA()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return false;
    }

    ASSERT_STATE("IsConnectedSTA");
    return m_pimpl->IsConnectedSTA();
}

bool P2PConMan::IsConnectedGO(void)
{
    QCC_DbgPrintf(("P2PConMan::IsConnectedGO()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return false;
    }

    ASSERT_STATE("IsConnectedGO");
    return m_pimpl->IsConnectedGO();
}

QStatus P2PConMan::CreateConnectSpec(const qcc::String& device, const qcc::String& guid, qcc::String& spec)
{
    QCC_DbgPrintf(("P2PConMan::CreateConnectSpec()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("CreateConnectSpec");
    return m_pimpl->CreateConnectSpec(device, guid, spec);
}

} // namespace ajn
