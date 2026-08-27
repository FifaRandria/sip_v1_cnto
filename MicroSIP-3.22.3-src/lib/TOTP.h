#pragma once

#include <string>

// TOTP (RFC 6238) - Time-based One-Time Password.
// Self-contained implementation (Base32 + HMAC-SHA1), no external dependency.
namespace TOTP {

// Generates the current TOTP code from a Base32 secret.
std::string generate(const std::string& secretBase32, uint64_t timeStep = 30, int digits = 6);

// Verifies a user entered code against the secret. Accepts a sliding window
// of +/- `window` time steps to tolerate clock drift between the
// authenticator app and this computer.
bool verify(const std::string& secretBase32, const std::string& code, uint64_t timeStep = 30, int digits = 6, int window = 1);

}
