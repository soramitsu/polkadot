/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "host_api/impl/misc_extension.hpp"

#include "runtime/binaryen/core_factory.hpp"
#include "runtime/common/const_wasm_provider.hpp"
#include "runtime/wasm_memory.hpp"
#include "scale/scale.hpp"

namespace kagome::host_api {

  MiscExtension::MiscExtension(
      uint64_t chain_id,
      std::shared_ptr<runtime::binaryen::CoreFactory> core_factory,
      std::shared_ptr<runtime::binaryen::RuntimeEnvironmentFactory>
          runtime_env_factory,
      std::shared_ptr<runtime::WasmMemory> memory)
      : core_api_factory_{std::move(core_factory)},
        runtime_env_factory_{std::move(runtime_env_factory)},
        memory_{std::move(memory)},
        logger_{common::createLogger("MiscExtension")},
        chain_id_{chain_id} {
    BOOST_ASSERT(core_api_factory_);
    BOOST_ASSERT(runtime_env_factory_);
    BOOST_ASSERT(memory_);
  }

  uint64_t MiscExtension::ext_chain_id() const {
    return chain_id_;
  }

  auto getRuntime() {
    common::Buffer buf(189, 0);
    buf[0] = 1;
    buf[1] = 233;
    buf[2] = 2;
    buf[3] = 32;
    buf[4] = 112;
    buf[5] = 111;
    buf[6] = 108;
    buf[7] = 107;
    buf[8] = 97;
    buf[9] = 100;
    buf[10] = 111;
    buf[11] = 116;
    buf[12] = 60;
    buf[13] = 112;
    buf[14] = 97;
    buf[15] = 114;
    buf[16] = 105;
    buf[17] = 116;
    buf[18] = 121;
    buf[19] = 45;
    buf[20] = 112;
    buf[21] = 111;
    buf[22] = 108;
    buf[23] = 107;
    buf[24] = 97;
    buf[25] = 100;
    buf[26] = 111;
    buf[27] = 116;
    buf[28] = 0;
    buf[29] = 0;
    buf[30] = 0;
    buf[31] = 0;
    buf[32] = 1;
    buf[33] = 0;
    buf[34] = 0;
    buf[35] = 0;
    buf[36] = 0;
    buf[37] = 0;
    buf[38] = 0;
    buf[39] = 0;
    buf[40] = 48;
    buf[41] = 223;
    buf[42] = 106;
    buf[43] = 203;
    buf[44] = 104;
    buf[45] = 153;
    buf[46] = 7;
    buf[47] = 96;
    buf[48] = 155;
    buf[49] = 3;
    buf[50] = 0;
    buf[51] = 0;
    buf[52] = 0;
    buf[53] = 55;
    buf[54] = 227;
    buf[55] = 151;
    buf[56] = 252;
    buf[57] = 124;
    buf[58] = 145;
    buf[59] = 245;
    buf[60] = 228;
    buf[61] = 1;
    buf[62] = 0;
    buf[63] = 0;
    buf[64] = 0;
    buf[65] = 64;
    buf[66] = 254;
    buf[67] = 58;
    buf[68] = 212;
    buf[69] = 1;
    buf[70] = 248;
    buf[71] = 149;
    buf[72] = 154;
    buf[73] = 4;
    buf[74] = 0;
    buf[75] = 0;
    buf[76] = 0;
    buf[77] = 210;
    buf[78] = 188;
    buf[79] = 152;
    buf[80] = 151;
    buf[81] = 238;
    buf[82] = 208;
    buf[83] = 143;
    buf[84] = 21;
    buf[85] = 2;
    buf[86] = 0;
    buf[87] = 0;
    buf[88] = 0;
    buf[89] = 247;
    buf[90] = 139;
    buf[91] = 39;
    buf[92] = 139;
    buf[93] = 229;
    buf[94] = 63;
    buf[95] = 69;
    buf[96] = 76;
    buf[97] = 2;
    buf[98] = 0;
    buf[99] = 0;
    buf[100] = 0;
    buf[101] = 175;
    buf[102] = 44;
    buf[103] = 2;
    buf[104] = 151;
    buf[105] = 162;
    buf[106] = 62;
    buf[107] = 109;
    buf[108] = 61;
    buf[109] = 3;
    buf[110] = 0;
    buf[111] = 0;
    buf[112] = 0;
    buf[113] = 237;
    buf[114] = 153;
    buf[115] = 197;
    buf[116] = 172;
    buf[117] = 178;
    buf[118] = 94;
    buf[119] = 237;
    buf[120] = 245;
    buf[121] = 2;
    buf[122] = 0;
    buf[123] = 0;
    buf[124] = 0;
    buf[125] = 203;
    buf[126] = 202;
    buf[127] = 37;
    buf[128] = 227;
    buf[129] = 159;
    buf[130] = 20;
    buf[131] = 35;
    buf[132] = 135;
    buf[133] = 2;
    buf[134] = 0;
    buf[135] = 0;
    buf[136] = 0;
    buf[137] = 104;
    buf[138] = 122;
    buf[139] = 212;
    buf[140] = 74;
    buf[141] = 211;
    buf[142] = 127;
    buf[143] = 3;
    buf[144] = 194;
    buf[145] = 1;
    buf[146] = 0;
    buf[147] = 0;
    buf[148] = 0;
    buf[149] = 171;
    buf[150] = 60;
    buf[151] = 5;
    buf[152] = 114;
    buf[153] = 41;
    buf[154] = 31;
    buf[155] = 235;
    buf[156] = 139;
    buf[157] = 1;
    buf[158] = 0;
    buf[159] = 0;
    buf[160] = 0;
    buf[161] = 188;
    buf[162] = 157;
    buf[163] = 137;
    buf[164] = 144;
    buf[165] = 79;
    buf[166] = 91;
    buf[167] = 146;
    buf[168] = 63;
    buf[169] = 1;
    buf[170] = 0;
    buf[171] = 0;
    buf[172] = 0;
    buf[173] = 55;
    buf[174] = 200;
    buf[175] = 187;
    buf[176] = 19;
    buf[177] = 80;
    buf[178] = 169;
    buf[179] = 162;
    buf[180] = 168;
    buf[181] = 1;
    buf[182] = 0;
    buf[183] = 0;
    buf[184] = 0;
    buf[185] = 0;
    buf[186] = 0;
    buf[187] = 0;
    buf[188] = 0;
    return buf;
  }

  runtime::WasmResult MiscExtension::ext_misc_runtime_version_version_1(
      runtime::WasmSpan data) const {
    auto [ptr, len] = runtime::splitSpan(data);
    auto code = memory_->loadN(ptr, len);
    auto wasm_provider =
        std::make_shared<runtime::ConstWasmProvider>(code);

    runtime_env_factory_->setIsolatedCode(code);

    auto core =
        core_api_factory_->createWithCode(runtime_env_factory_, wasm_provider);
    auto version_res = core->version(boost::none);

    static const auto error_res =
        scale::encode<boost::optional<primitives::Version>>(boost::none)
            .value();

    if (version_res.has_value()) {
      auto enc_version_res =
          scale::encode(boost::make_optional(version_res.value()));
      if (enc_version_res.has_error()) {
        logger_->error(
            "Error encoding ext_misc_runtime_version_version_1 result: {}",
            enc_version_res.error().message());
        return runtime::WasmResult{memory_->storeBuffer(error_res)};
      }
      auto fake_version =
          getRuntime();
      auto res_span = memory_->storeBuffer(fake_version);
      return runtime::WasmResult{res_span};
    }
    logger_->error("Error inside Core_version: {}",
                   version_res.error().message());
    return runtime::WasmResult{memory_->storeBuffer(error_res)};
  }

  void MiscExtension::ext_misc_print_hex_version_1(
      runtime::WasmSpan data) const {
    auto [ptr, len] = runtime::splitSpan(data);
    auto buf = memory_->loadN(ptr, len);
    logger_->info("hex: {}", buf.toHex());
  }

  void MiscExtension::ext_misc_print_num_version_1(uint64_t value) const {
    logger_->info("num: {}", value);
  }

  void MiscExtension::ext_misc_print_utf8_version_1(
      runtime::WasmSpan data) const {
    auto [ptr, len] = runtime::splitSpan(data);
    auto buf = memory_->loadN(ptr, len);
    logger_->info("utf8: {}", buf.toString());
  }

}  // namespace kagome::host_api
