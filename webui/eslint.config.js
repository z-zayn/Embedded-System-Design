import js from "@eslint/js";
import vue from "eslint-plugin-vue";
import tseslint from "typescript-eslint";
import vueParser from "vue-eslint-parser";

import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let autoImportGlobals = {};
try {
  const p = path.join(__dirname, ".eslintrc-auto-import.json");
  const json = JSON.parse(fs.readFileSync(p, "utf8"));
  autoImportGlobals = json.globals ?? {};
} catch {
  // 文件不存在或解析失败时忽略（例如首次运行还没生成）
  autoImportGlobals = {};
}

export default [
  {
    ignores: ["dist/**", "build/**", "coverage/**", "node_modules/**"],
  },

  js.configs.recommended,
  ...tseslint.configs.recommended,

  // Vue（SFC）规则
  ...vue.configs["flat/recommended"],

  // 针对 .vue 做 parser 串联：vue-eslint-parser + typescript-eslint
  {
    files: ["**/*.vue"],
    languageOptions: {
      parser: vueParser,
      parserOptions: {
        parser: tseslint.parser,
        ecmaVersion: "latest",
        sourceType: "module",
        extraFileExtensions: [".vue"],
      },
      globals: {
        ...autoImportGlobals,
      },
    },
    rules: {
      "vue/multi-word-component-names": "off",
    },
  },

  // 针对 .ts/.tsx
  {
    files: ["**/*.ts", "**/*.tsx"],
    languageOptions: {
      parser: tseslint.parser,
    },
    rules: {
      "@typescript-eslint/no-unused-vars": ["warn", { argsIgnorePattern: "^_" }],
      "vue/max-attributes-per-line": "off",
      "vue/first-attribute-linebreak": "off",
    },
  },
];
