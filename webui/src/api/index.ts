export type StatusResp = {
  uptime: number
  load1: number
  memTotalKb: number
  memAvailKb: number
}

export type FileItem = {
  name: string
  size: number
  mtime: number
}

export type CtlResp = {
  ok: boolean
  msg: string
  running?: boolean
  pid?: number
}

async function getJson<T>(url: string): Promise<T> {
  const res = await fetch(url, { cache: 'no-store' })
  if (!res.ok) throw new Error(`${res.status} ${res.statusText}`)
  return await res.json() as T
}

export const api = {
  status: () => getJson<StatusResp>('/cgi-bin/status.cgi'),
  files: () => getJson<FileItem[]>('/cgi-bin/files.cgi'),
  ctlStatus: () => getJson<CtlResp>('/cgi-bin/ctl.cgi?action=status'),
  ctlStart: () => getJson<CtlResp>('/cgi-bin/ctl.cgi?action=start'),
  ctlStop: () => getJson<CtlResp>('/cgi-bin/ctl.cgi?action=stop'),
}
