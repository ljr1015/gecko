/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_WebAuthnCoseIdentifiers_h
#define mozilla_dom_WebAuthnCoseIdentifiers_h

#include "mozilla/dom/WebCryptoCommon.h"

namespace mozilla {
namespace dom {

// From https://www.iana.org/assignments/cose/cose.xhtml#algorithms
enum class CoseAlgorithmIdentifier : int32_t {
  ES256 = -7
};

static nsresult
CoseAlgorithmToWebCryptoId(const int32_t& aId, /* out */ nsString& aName)
{
  switch(static_cast<CoseAlgorithmIdentifier>(aId)) {
    case CoseAlgorithmIdentifier::ES256:
      aName.AssignLiteral(JWK_ALG_ECDSA_P_256);
      break;
    default:
      return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
  }
  return NS_OK;
}

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_WebAuthnCoseIdentifiers_h
