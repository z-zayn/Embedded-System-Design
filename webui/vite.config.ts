import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import AutoImport from "unplugin-auto-import/vite"
import Components from "unplugin-vue-components/vite"
import { ElementPlusResolver } from "unplugin-vue-components/resolvers"

export default defineConfig({
  plugins: [vue(),
    AutoImport({
      imports: ["vue"],
      dts: "src/auto-imports.d.ts",
      resolvers: [
        ElementPlusResolver(), // 关键：让 ElMessage/ElNotification 等可自动导入
      ],
      // 生成 ESLint 配置，避免 no-undef（推荐）
      eslintrc: {
        enabled: true,
        filepath: "./.eslintrc-auto-import.json",
        globalsPropValue: true
      }
    }),
    Components({
      resolvers: [ElementPlusResolver()],
      dts: "src/components.d.ts",
    }),
  ],
  base: './',
  server: {
    proxy: {
      '/cgi-bin': {
        target: 'http://127.0.0.1:8080',
        changeOrigin: true,
      },
    },
  },
})