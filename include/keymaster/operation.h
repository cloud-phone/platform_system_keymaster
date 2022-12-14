/*
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SYSTEM_KEYMASTER_OPERATION_H_
#define SYSTEM_KEYMASTER_OPERATION_H_

#include <utility>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <hardware/keymaster_defs.h>
#include <keymaster/android_keymaster_utils.h>
#include <keymaster/authorization_set.h>
#include <keymaster/logger.h>

namespace keymaster {

class AuthorizationSet;
class Key;
class Operation;
using OperationPtr = UniquePtr<Operation>;

class OperationFactory {
  public:
    virtual ~OperationFactory() {}

    // Required for registry
    struct KeyType {
        KeyType(keymaster_algorithm_t alg, keymaster_purpose_t purp)
            : algorithm(alg), purpose(purp) {}

        keymaster_algorithm_t algorithm;
        keymaster_purpose_t purpose;

        bool operator==(const KeyType& rhs) const {
            return algorithm == rhs.algorithm && purpose == rhs.purpose;
        }
    };
    virtual KeyType registry_key() const = 0;

    // Factory methods
    virtual OperationPtr CreateOperation(Key&& key, const AuthorizationSet& begin_params,
                                         keymaster_error_t* error) = 0;

    // Informational methods.  The returned arrays reference static memory and must not be
    // deallocated or modified.
    virtual const keymaster_padding_t* SupportedPaddingModes(size_t* padding_count) const {
        *padding_count = 0;
        return nullptr;
    }
    virtual const keymaster_block_mode_t* SupportedBlockModes(size_t* block_mode_count) const {
        *block_mode_count = 0;
        return nullptr;
    }
    virtual const keymaster_digest_t* SupportedDigests(size_t* digest_count) const {
        *digest_count = 0;
        return nullptr;
    }

    // Convenience methods
    bool supported(keymaster_padding_t padding) const;
    bool supported(keymaster_block_mode_t padding) const;
    bool supported(keymaster_digest_t padding) const;

    bool is_public_key_operation() const;

    bool GetAndValidatePadding(const AuthorizationSet& begin_params, const Key& key,
                               keymaster_padding_t* padding, keymaster_error_t* error) const;
    bool GetAndValidateDigest(const AuthorizationSet& begin_params, const Key& key,
                              keymaster_digest_t* digest, keymaster_error_t* error) const;
    bool GetAndValidateDigest(const AuthorizationSet& begin_params, const Key& key,
                              keymaster_digest_t* digest, keymaster_error_t* error,
                              bool require_explicit_digest) const;
};

/**
 * Abstract base for all cryptographic operations.
 */
class Operation {
  public:
    explicit Operation(keymaster_purpose_t purpose, AuthorizationSet&& hw_enforced,
                       AuthorizationSet&& sw_enforced)
        : purpose_(purpose), hw_enforced_(std::move(hw_enforced)),
          sw_enforced_(std::move(sw_enforced)) {}
    virtual ~Operation() {}

    Operation(const Operation&) = delete;
    void operator=(const Operation&) = delete;

    keymaster_purpose_t purpose() const { return purpose_; }

    void set_key_id(uint64_t key_id) { key_id_ = key_id; }
    uint64_t key_id() const { return key_id_; }
    void set_secure_deletion_slot(uint32_t secure_deletion_slot) {
        secure_deletion_slot_ = secure_deletion_slot;
    }
    uint32_t secure_deletion_slot() const { return secure_deletion_slot_; }
    virtual keymaster_operation_handle_t operation_handle() const { return operation_handle_; }

    AuthProxy authorizations() const { return AuthProxy(hw_enforced_, sw_enforced_); }
    AuthorizationSet hw_enforced() const { return hw_enforced_; }
    AuthorizationSet sw_enforced() const { return sw_enforced_; }

    // Creates and initializes |confirmation_verifier_buffer_| that can be retrieved with
    // get_confirmation_verifier_buffer().
    //
    // Returns false on allocation failure.
    bool create_confirmation_verifier_buffer() {
        if (!confirmation_verifier_buffer_) {
            Buffer* buffer = new (std::nothrow)
                Buffer(kConfirmationTokenMessageTag, kConfirmationTokenMessageTagSize);
            if (buffer == nullptr) {
                return false;
            }
            confirmation_verifier_buffer_.reset(buffer);
        }
        return true;
    }

    // If a Buffer for ConfirmationUI verification was created with
    // create_confirmation_verifier_buffer(), returns it. If not, returns |nullptr|.
    Buffer* get_confirmation_verifier_buffer() { return confirmation_verifier_buffer_.get(); }

    virtual keymaster_error_t Begin(const AuthorizationSet& input_params,
                                    AuthorizationSet* output_params) = 0;
    virtual keymaster_error_t Update(const AuthorizationSet& input_params, const Buffer& input,
                                     AuthorizationSet* output_params, Buffer* output,
                                     size_t* input_consumed) = 0;
    virtual keymaster_error_t Finish(const AuthorizationSet& input_params, const Buffer& input,
                                     const Buffer& signature, AuthorizationSet* output_params,
                                     Buffer* output) = 0;
    virtual keymaster_error_t Abort() = 0;

  protected:
    // Helper function for implementing Finish() methods that need to call Update() to process
    // input, but don't expect any output.
    keymaster_error_t UpdateForFinish(const AuthorizationSet& input_params, const Buffer& input);
    keymaster_operation_handle_t operation_handle_;

  private:
    const keymaster_purpose_t purpose_;
    AuthorizationSet hw_enforced_;
    AuthorizationSet sw_enforced_;
    uint64_t key_id_;
    uint32_t secure_deletion_slot_ = 0;
    UniquePtr<Buffer> confirmation_verifier_buffer_;
};

}  // namespace keymaster

#endif  // SYSTEM_KEYMASTER_OPERATION_H_
