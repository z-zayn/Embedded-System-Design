<template>
  <el-card style="margin-top: 16px">
    <template #header>
      <span>Last Operation</span>
    </template>

    <el-alert
      v-if="lastMsg"
      :title="lastMsg"
      type="info"
      show-icon
    />
    <pre class="pre">{{ lastRaw }}</pre>
  </el-card>
</template>

<script setup lang="ts">
defineProps<{
  lastMsg: string;
  lastRaw: string;
}>();
</script>

<style scoped lang="scss">
.pre {
  background: #0b1020;
  color: #d6e1ff;
  padding: 12px;
  border-radius: 6px;
  overflow: auto;
}
</style>
