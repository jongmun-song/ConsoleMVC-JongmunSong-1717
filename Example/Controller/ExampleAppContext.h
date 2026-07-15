#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7).
//
// ExampleAppContext bundles the in-memory storage the example menu tree
// (ExampleMenuTreeFactory.h) operates on: one InMemoryModel<T> per entity
// type plus the single global production FifoQueue, exactly as
// docs/feature/model.md section 6 and ../../CLAUDE.md ("영속성 없음") specify -
// no persistence adapter is supplied, so everything lives only in memory for
// the process lifetime.
//
// It also owns simple monotonically increasing id counters. IEntity<TId>
// requires ids to be assigned once at construction and never change
// afterwards (see ../../Model/IEntity.h) - since this skeleton's core does
// not provide an id generator (that is a domain concern), the example
// context supplies the simplest one that satisfies that contract.
//
// Monitoring extension point demo (docs/PLAN.md section 3 / docs/design/
// phase4.md item 2): sampleModel/orderModel each get a
// ConsoleMVC::Example::Model::ChangeLogObserver<T> subscribed here, at
// construction time, purely through the core IModel<T>::Subscribe hook -
// SampleMenuFactory/OrderMenuFactory/.../ExampleMenuTreeFactory (Controller/
// and View/, core or Example/) are never touched to make this observation
// happen. m_sampleChangeLog/m_orderChangeLog are declared before
// sampleModel/orderModel so the observers already exist by the time the
// models attempt to construct (member destruction order - reverse of
// declaration - also unsubscribes them before the models are destroyed).

#include "../Model/ChangeLogObserver.h"
#include "../Model/Order.h"
#include "../Model/ProductionQueueEntry.h"
#include "../Model/Sample.h"
#include "../../Model/FifoQueue.h"
#include "../../Model/InMemoryModel.h"

#include <sstream>
#include <string>

namespace ConsoleMVC::Example::Controller
{
    struct ExampleAppContext
    {
        ExampleAppContext()
        {
            sampleModel.Subscribe(&sampleChangeLog);
            orderModel.Subscribe(&orderChangeLog);
        }

        ~ExampleAppContext()
        {
            sampleModel.Unsubscribe(&sampleChangeLog);
            orderModel.Unsubscribe(&orderChangeLog);
        }

        ExampleAppContext(const ExampleAppContext&) = delete;
        ExampleAppContext& operator=(const ExampleAppContext&) = delete;

        Model::ChangeLogObserver<Model::Sample> sampleChangeLog{
            "Sample",
            [](const Model::Sample& sample)
            {
                std::ostringstream description;
                description << "#" << sample.GetId() << " '" << sample.GetName()
                            << "' stock=" << sample.GetStockQuantity()
                            << " reserved=" << sample.GetReservedQuantity();
                return description.str();
            }};

        Model::ChangeLogObserver<Model::Order> orderChangeLog{
            "Order",
            [](const Model::Order& order)
            {
                std::ostringstream description;
                description << "#" << order.GetId() << " sample=" << order.GetSampleId()
                            << " qty=" << order.GetOrderedQuantity()
                            << " state=" << static_cast<int>(order.GetState());
                return description.str();
            }};

        ConsoleMVC::Model::InMemoryModel<Model::Sample> sampleModel;
        ConsoleMVC::Model::InMemoryModel<Model::Order> orderModel;
        ConsoleMVC::Model::FifoQueue<Model::ProductionQueueEntry> productionQueue;

        int nextSampleId = 1;
        int nextOrderId = 1;

        int TakeNextSampleId()
        {
            return nextSampleId++;
        }

        int TakeNextOrderId()
        {
            return nextOrderId++;
        }
    };
}
