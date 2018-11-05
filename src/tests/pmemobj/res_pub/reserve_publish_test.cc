/*
 * Copyright (c) 2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * * Neither the name of Intel Corporation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "reserve_publish.h"
#include <thread>
#include <future>

/**
 * RESERVE_PUBLISH_PUBLISH
 * Parameterized Test Case: Checks that objects are correctly populated and
 * allocated. Parameters are:
 *  - the number of threads (n)
 *  - the size of the objects to be reserved
 *  - the number of objects to be reserved by each thread
 * \test
 *          \li \c Step1 Create the pmemobj pool file
 *          \li \c Step2 Make reservations with pmemobj_reserve using n threads
 *          \li \c Step3 Synchronize the threads
 *          \li \c Step4 Publish the objects with pmemobj_publish
 *          \li \c Step5 Close, check and reopen the pool
 *          \li \c Step6 Verify that the objects were allocated
 *          \li \c Step7 Close and remove the pool
 */
TEST_P(PmemobjReservePublishParamTest, RESERVE_PUBLISH_PUBLISH) {
  std::vector<std::thread> threads;
  std::vector<std::promise<std::unique_ptr<ActionsObj>>> promise_objects(nof_threads);
  std::vector<std::future<std::unique_ptr<ActionsObj>>> future_objects;

  std::vector<struct pobj_action> reservations;

  size_t nof_messages = nof_threads * messages_per_thread;

  /* Step 1 */
  pop = pmemobj_create(pool_path_.c_str(), LAYOUT_NAME, pool_size,
    S_IWRITE | S_IREAD);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 2 */
  for (int th = 0; th < nof_threads; th++) {
    future_objects.push_back(std::move(promise_objects[th].get_future()));
    threads.push_back(std::thread(&PmemobjReservePublishTest::ReserveInThread,
      this, std::move(promise_objects[th])));
  }

  for (int th = 0; th < nof_threads; th++) {
    std::unique_ptr<ActionsObj> future_object = future_objects[th].get();
    reservations.insert(
      reservations.end(),
      future_object->publish_acts.begin(),
      future_object->publish_acts.end());
  }

  /* Step 3 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 4 */
  int ret = pmemobj_publish(pop, &reservations[0], nof_messages);
  ASSERT_EQ(0, ret);

  /* Step 5 */
  pmemobj_close(pop);
  ASSERT_EQ(1, pmemobj_check(pool_path_.c_str(), LAYOUT_NAME));
  pop = pmemobj_open(pool_path_.c_str(), LAYOUT_NAME);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 6 */
  ASSERT_EQ(nof_messages, GetNofStoredMessages());
}

/**
 * RESERVE_PUBLISH_CANCEL
 * Parameterized Test Case: Checks that objects are indeed not allocated if the
 * reservation is cancelled. Parameters are:
 *  - the number of threads (n)
 *  - the size of the objects to be reserved
 * \test
 *          \li \c Step1 Create the pmemobj pool file
 *          \li \c Step2 Make reservations until the entire pool is filled
 *          \li \c Step3 Cancel all reserved objects
 *          \li \c Step4 Make reservations using n threads
 *          \li \c Step5 Synchronize the threads
 *          \li \c Step6 Make additional reservations until the entire pool is
 *          filled
 *          \li \c Step7 Verify that the number of reservations made in step 2
 *          is equal to the number of reservations made in steps 4 and 6 together
 *          \li \c Step8 Cancel every 4th object that was reserved in Step 4
 *          \li \c Step9 Close, check and reopen the pool
 *          \li \c Step10 Make reservations until the entire pool is filled
 *          \li \c Step11 Verify that the number of reservations made in Step10
 *          equals the number of cancellations in Step8
 *          \li \c Step12 Close and remove the pool
 */
TEST_P(PmemobjReservePublishParamTest, RESERVE_PUBLISH_CANCEL) {
  std::vector<std::thread> threads;
  std::vector<std::promise<std::unique_ptr<ActionsObj>>> promise_objects(nof_threads);
  std::vector<std::future<std::unique_ptr<ActionsObj>>> future_objects;

  std::vector<struct pobj_action> reservations;
  std::vector<struct pobj_action> cancellations;

  size_t nof_messages = nof_threads * messages_per_thread;

  /* Step 1 */
  pop = pmemobj_create(pool_path_.c_str(), LAYOUT_NAME, pool_size,
    S_IWRITE | S_IREAD);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 2 */
  std::vector<pobj_action> actual_messages = MakeMaximumReservations();
  size_t actual_nof_messages = actual_messages.size();

  /* Step 3 */
  pmemobj_cancel(pop, &actual_messages[0], actual_nof_messages);
  actual_messages.clear();

  messages_per_thread = actual_nof_messages / nof_threads;

  /* Step 4 */
  for (int th = 0; th < nof_threads; th++) {
    future_objects.push_back(std::move(promise_objects[th].get_future()));
    threads.push_back(std::thread(&PmemobjReservePublishTest::ReserveWithCancelInThread, 
      this, std::move(promise_objects[th])));
  }

  for (int th = 0; th < nof_threads; th++) {
    std::unique_ptr<ActionsObj> future_object = future_objects[th].get();
    reservations.insert(
      reservations.end(),
      future_object->publish_acts.begin(),
      future_object->publish_acts.end());
    cancellations.insert(
      cancellations.end(),
      future_object->cancel_acts.begin(),
      future_object->cancel_acts.end());
  }

  /* Step 5 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 6 */
  std::vector<pobj_action> remaining_messages = MakeMaximumReservations();
  size_t remaining_space = remaining_messages.size();

  /* Step 7 */
  ASSERT_EQ(actual_nof_messages, reservations.size() + cancellations.size() + remaining_space);

  int ret = pmemobj_publish(pop, &reservations[0], reservations.size());
  ASSERT_EQ(0, ret) << pmemobj_errormsg();

  if (remaining_space > 0) {
    ret = pmemobj_publish(pop, &remaining_messages[0], remaining_space);
    ASSERT_EQ(0, ret) << pmemobj_errormsg();
  }

  /* Step 8 */
  int nof_cancellations = cancellations.size();
  pmemobj_cancel(pop, &cancellations[0], nof_cancellations);

  /* Step 9 */
  pmemobj_close(pop);
  ASSERT_EQ(1, pmemobj_check(pool_path_.c_str(), LAYOUT_NAME));
  pop = pmemobj_open(pool_path_.c_str(), LAYOUT_NAME);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 10 */
  std::vector<pobj_action> freed_messages = MakeMaximumReservations();

  /* Step 11 */
  ASSERT_EQ(nof_cancellations, freed_messages.size());
}

/**
 * RESERVE_PUBLISH_DEFER_FREE_01
 * Parameterized Test Case: Verifies that objects are freed after publishing
 * defer_free actions
 *  - the number of threads (n)
 *  - the size of the objects to be reserved
 *  - the number of objects to be reserved by each thread
 * \test
 *          \li \c Step1 Create the pmemobj pool file
 *          \li \c Step2 Allocate objects with pmemobj_alloc and mark every 2nd
 *          object to be freed with pmemobj_defer_free, using n threads
 *          \li \c Step3 Synchronize the threads
 *          \li \c Step4 Make additional reservations until the entire pool is filled
 *          \li \c Step5 Publish the free actions with pmemobj_publish
 *          \li \c Step6 Close, check and reopen the pool
 *          \li \c Step7 Verify that the expected number of objects were freed in step 6
 *          \li \c Step8 Close and remove the pool
 */
TEST_P(PmemobjReservePublishParamTest, RESERVE_PUBLISH_DEFER_FREE_01) {
  std::vector<std::thread> threads;
  std::vector<std::promise<std::unique_ptr<ActionsObj>>> promise_objects(nof_threads);
  std::vector<std::future<std::unique_ptr<ActionsObj>>> future_objects;

  std::vector<struct pobj_action> defer_frees;

  /* Step 1 */
  pop = pmemobj_create(pool_path_.c_str(), LAYOUT_NAME, pool_size,
    S_IWRITE | S_IREAD);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 2 */
  for (int th = 0; th < nof_threads; th++) {
    future_objects.push_back(std::move(promise_objects[th].get_future()));
    threads.push_back(std::thread(&PmemobjReservePublishTest::DeferFreeInThread, 
      this, std::move(promise_objects[th])));
  }

  for (int th = 0; th < nof_threads; th++) {
    std::unique_ptr<ActionsObj> future_object = future_objects[th].get();
    defer_frees.insert(
      defer_frees.end(),
      future_object->publish_acts.begin(),
      future_object->publish_acts.end());
  }

  /* Step 3 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 4 */
  std::vector<pobj_action> remaining_messages = MakeMaximumReservations();

  if (remaining_messages.size() > 0) {
    int ret = pmemobj_publish(pop, &remaining_messages[0], remaining_messages.size());
    ASSERT_EQ(0, ret) << pmemobj_errormsg();
  }

  /* Step 5 */
  size_t nof_frees = defer_frees.size();
  int ret = pmemobj_publish(pop, &defer_frees[0], nof_frees);
  ASSERT_EQ(0, ret);

  /* Step 6 */
  pmemobj_close(pop);
  ASSERT_EQ(1, pmemobj_check(pool_path_.c_str(), LAYOUT_NAME));
  pop = pmemobj_open(pool_path_.c_str(), LAYOUT_NAME);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 7 */
  std::vector<pobj_action> free_space = MakeMaximumReservations();
  ASSERT_EQ(free_space.size(), nof_frees);
}

/**
 * RESERVE_PUBLISH_DEFER_FREE_02
 * Parameterized Test Case: Verifies that objects are not freed when the
 * defer_free actions are cancelled
 *  - the number of threads (n)
 *  - the size of the objects to be reserved
 *  - the number of objects to be reserved by each thread
 * \test
 *          \li \c Step1 Create the pmemobj pool file
 *          \li \c Step2 Allocate objects with pmemobj_alloc and mark every 2nd
 *          object to be freed with pmemobj_defer_free, using n threads
 *          \li \c Step3 Synchronize the threads
 *          \li \c Step4 Make additional reservations until the entire pool is
 *          filled
 *          \li \c Step5 Cancel the free actions with pmemobj_cancel
 *          \li \c Step6 Close, check and reopen the pool
 *          \li \c Step7 Verify that none of the objects were freed
 *          \li \c Step8 Close and remove the pool
 */
TEST_P(PmemobjReservePublishParamTest, RESERVE_PUBLISH_DEFER_FREE_02) {
  std::vector<std::thread> threads;
  std::vector<std::promise<std::unique_ptr<ActionsObj>>> promise_objects(nof_threads);
  std::vector<std::future<std::unique_ptr<ActionsObj>>> future_objects;

  std::vector<struct pobj_action> defer_frees;

  /* Step 1 */
  pop = pmemobj_create(pool_path_.c_str(), LAYOUT_NAME, pool_size,
    S_IWRITE | S_IREAD);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  for (int th = 0; th < nof_threads; th++) {
    future_objects.push_back(std::move(promise_objects[th].get_future()));
    threads.push_back(std::thread(&PmemobjReservePublishTest::DeferFreeInThread, 
      this, std::move(promise_objects[th])));
  }

  /* Step 2 */
  for (int th = 0; th < nof_threads; th++) {
    std::unique_ptr<ActionsObj> future_object = future_objects[th].get();
    defer_frees.insert(
      defer_frees.end(),
      future_object->publish_acts.begin(),
      future_object->publish_acts.end());
  }

  /* Step 3 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 4 */
  std::vector<pobj_action> remaining_messages = MakeMaximumReservations();

  if (remaining_messages.size() > 0) {
    int ret = pmemobj_publish(pop, &remaining_messages[0], remaining_messages.size());
    ASSERT_EQ(0, ret) << pmemobj_errormsg();
  }

  /* Step 5 */
  size_t nof_frees = defer_frees.size();
  pmemobj_cancel(pop, &defer_frees[0], nof_frees);

  /* Step 6 */
  pmemobj_close(pop);
  ASSERT_EQ(1, pmemobj_check(pool_path_.c_str(), LAYOUT_NAME));
  pop = pmemobj_open(pool_path_.c_str(), LAYOUT_NAME);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 7 */
  std::vector<pobj_action> free_space = MakeMaximumReservations();
  ASSERT_EQ(free_space.size(), 0);
}

/**
 * RESERVE_PUBLISH_XRESERVE_01
 * Parameterized Test Case: Checks the functionality of pmemobj_xreserve using
 * the POBJ_XALLOC_ZERO flag for the following parameters:
 *  - the number of threads (n)
 *  - the size of the objects to be reserved
 *  - the number of objects to be reserved by each thread
 * \test
 *          \li \c Step1 Create the pmemobj pool file
 *          \li \c Step2 Make reservations with pmemobj_xreserve with the flags 
 *          parameter set to POBJ_XALLOC_ZERO and using n threads
 *          \li \c Step3 Synchronize the threads
 *          \li \c Step4 Publish the reserved objects with pmemobj_xpublish
 *          using n threads
 *          \li \c Step5 Synchronize the threads
 *          \li \c Step6 Close, check and reopen the pool
 *          \li \c Step7 Verify that the objects were allocated
 *          \li \c Step8 Close and remove the pool
 */
TEST_P(PmemobjReservePublishParamTest, RESERVE_PUBLISH_XRESERVE_01) {
  std::vector<std::thread> threads;
  std::vector<std::promise<std::unique_ptr<ActionsObj>>> promise_objects(nof_threads);
  std::vector<std::future<std::unique_ptr<ActionsObj>>> future_objects;

  std::vector<struct TestObj> reservations;

  flags = POBJ_XALLOC_ZERO;
  size_t nof_messages = nof_threads * messages_per_thread;
  int ret;

  /* Step 1 */
  pop = pmemobj_create(pool_path_.c_str(), LAYOUT_NAME, pool_size,
    S_IWRITE | S_IREAD);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 2 */
  for (int th = 0; th < nof_threads; th++) {
    future_objects.push_back(std::move(promise_objects[th].get_future()));
    threads.push_back(std::thread(&PmemobjReservePublishTest::XReserveInThread,
      this, std::move(promise_objects[th])));
  }

  for (int th = 0; th < nof_threads; th++) {
    std::unique_ptr<ActionsObj> future_object = future_objects[th].get();
    reservations.push_back(TestObj(future_object->publish_acts));
  }

  /* Step 3 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 4 */
  for (int th = 0; th < nof_threads; th++) {
    threads.push_back(std::thread(&PmemobjReservePublishParamTest::TxPublishInThread,
      this, reservations[th]));
  }

  /* Step 5 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 6 */
  pmemobj_close(pop);
  ASSERT_EQ(1, pmemobj_check(pool_path_.c_str(), LAYOUT_NAME));
  pop = pmemobj_open(pool_path_.c_str(), LAYOUT_NAME);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 7 */
  ASSERT_EQ(nof_messages, GetNofStoredMessages());
}

/**
 * RESERVE_PUBLISH_XRESERVE_02
 * Parameterized Test Case: Checks the functionality of pmemobj_xreserve using
 * the POBJ_CLASS_ID flag for the following parameters:
 *  - the number of threads (n)
 *  - the size of the objects to be reserved
 *  - the number of objects to be reserved by each thread
 * \test
 *          \li \c Step1 Create the pmemobj pool file
 *          \li \c Step2 Register an allocation class and obtain the class ID
 *          \li \c Step3 Make reservations with pmemobj_xreserve with the flags 
 *          parameter set to the obtained class ID and using n threads
 *          \li \c Step4 Synchronize the threads
 *          \li \c Step5 Publish the reserved objects with pmemobj_xpublish
 *          using n threads
 *          \li \c Step6 Synchronize the threads
 *          \li \c Step7 Close, check and reopen the pool
 *          \li \c Step8 Verify that the objects were allocated
 *          \li \c Step9 Close and remove the pool
 */
TEST_P(PmemobjReservePublishParamTest, RESERVE_PUBLISH_XRESERVE_02) {
  std::vector<std::thread> threads;
  std::vector<std::promise<std::unique_ptr<ActionsObj>>> promise_objects(nof_threads);
  std::vector<std::future<std::unique_ptr<ActionsObj>>> future_objects;

  std::vector<struct TestObj> reservations;

  struct pobj_alloc_class_desc pacd;
  pacd.header_type = POBJ_HEADER_NONE;
  pacd.unit_size = data_size;
  pacd.units_per_block = 4;
  pacd.alignment = 0;

  /* Step 1 */
  pop = pmemobj_create(pool_path_.c_str(), LAYOUT_NAME, pool_size,
    S_IWRITE | S_IREAD);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 2 */
  int ret = pmemobj_ctl_set(pop, "heap.alloc_class.128.desc", &pacd);
  ASSERT_EQ(errno, 0);
  ASSERT_EQ(ret, 0) << pmemobj_errormsg();

  flags = pacd.class_id;
  size_t nof_messages = nof_threads * messages_per_thread;

  /* Step 3 */
  for (int th = 0; th < nof_threads; th++) {
    future_objects.push_back(std::move(promise_objects[th].get_future()));
    threads.push_back(std::thread(&PmemobjReservePublishTest::XReserveInThread,
      this, std::move(promise_objects[th])));
  }

  for (int th = 0; th < nof_threads; th++) {
    std::unique_ptr<ActionsObj> future_object = future_objects[th].get();
    reservations.push_back(TestObj(future_object->publish_acts));
  }

  /* Step 4 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 5 */
  for (int th = 0; th < nof_threads; th++) {
    threads.push_back(std::thread(&PmemobjReservePublishParamTest::TxPublishInThread,
      this, reservations[th]));
  }

  /* Step 6 */
  for (int th = 0; th < nof_threads; th++) {
    threads[th].join();
  }
  threads.clear();

  /* Step 7 */
  pmemobj_close(pop);
  ASSERT_EQ(1, pmemobj_check(pool_path_.c_str(), LAYOUT_NAME));
  pop = pmemobj_open(pool_path_.c_str(), LAYOUT_NAME);
  ASSERT_TRUE(pop != nullptr) << pmemobj_errormsg();

  /* Step 8 */
  ASSERT_EQ(nof_messages, GetNofStoredMessages());
}

INSTANTIATE_TEST_CASE_P(
    ResPubParam, PmemobjReservePublishParamTest,
    ::testing::Values(
        ReservePublishParams(2 * MEBIBYTE, 1, 8),
        ReservePublishParams(2 * MEBIBYTE, 8, 2),
        ReservePublishParams(2 * MEBIBYTE, 8, 4),
        ReservePublishParams(2 * MEBIBYTE, 8, 8)));
