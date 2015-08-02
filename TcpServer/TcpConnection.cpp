#include "TcpConnection.h"
#include "Sock.h"

TcpConnection::TcpConnection(SOCKET iFd)
{
    m_socket = iFd;
    m_llClientHandle = 0;
    m_bSendOver = false;
    m_bCloseByUser = false;
    m_SendOpContext.m_IoType = OP_Send;
    m_RecvOpContext.m_IoType = OP_Recv;
    m_isRecving = false;
    m_isSending = false;
    m_isClosing = false;
}

TcpConnection::~TcpConnection()
{

}

SOCKET TcpConnection::GetFd()
{
    return m_socket;
}

void TcpConnection::DestroySocket()
{
    Sock::CloseFd(m_socket);
}

void TcpConnection::SetHandle(int64_t llClientHandle)
{
    m_llClientHandle = llClientHandle;
}

int64_t TcpConnection::GetHandle()
{
    return m_llClientHandle;
}

void TcpConnection::AppendMessage(const void *data, int iLen)
{
    if (!data || iLen <= 0)
    {
        return;
    }

    //uint32_t uiNetLen = htonl((uint32_t)iLen);
    int32_t iHead = sizeof(int32_t) + (int32_t)iLen; // 头部4字节长度+body长度

    //LOGINFO<<"client["<<m_llClientHandle<<"] want send "<<iHead<<" bytes data";

    std::lock_guard<std::mutex> lockGuard(m_SendBufferlock);
    m_vecTmpSendBuffer.insert(m_vecTmpSendBuffer.end(), (char *)(&iHead), (char *)(&iHead) + sizeof(iHead));
    m_vecTmpSendBuffer.insert(m_vecTmpSendBuffer.end(), (char *)data, (char *)data + iLen);    
}

bool TcpConnection::PostSend()
{
    m_SendBufferlock.lock();

    if (!m_vecTmpSendBuffer.empty())
    {
        m_vecSendBuffer.insert(m_vecSendBuffer.end(), m_vecTmpSendBuffer.begin(),
            m_vecTmpSendBuffer.end());
    }

    m_SendBufferlock.unlock();

    if (!m_vecSendBuffer.empty())
    {
        int len = min(POST_SIZE, m_vecSendBuffer.size());

        m_SendOpContext.wsaBuffer.buf = m_vecSendBuffer.data();        
        m_SendOpContext.wsaBuffer.len = len;

        DWORD dwFlag = 0;
        if (WSASend(m_socket, &m_SendOpContext.wsaBuffer, 1, 
            &dwFlag, 0, (LPWSAOVERLAPPED)&m_SendOpContext, NULL) != 0)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                return false;
            }
        }

        m_isSending = true;
    }

    return true;
}

void TcpConnection::NotifySendHowMuchBytes(int iTranceCount)
{
    std::lock_guard<std::mutex> lockGuard(m_SendBufferlock);
    m_vecSendBuffer.erase(m_vecSendBuffer.begin(), m_vecSendBuffer.begin() + iTranceCount);
    if (!m_vecTmpSendBuffer.empty())
    {
        m_vecSendBuffer.insert(m_vecSendBuffer.end(), m_vecTmpSendBuffer.begin(),
            m_vecTmpSendBuffer.end());
    }

    if (m_vecSendBuffer.empty())
    {
        m_bSendOver=true;
    }
}

bool TcpConnection::PostRead()
{
    m_RecvOpContext.wsaBuffer.buf = m_tmpRecvBuffer;
    m_RecvOpContext.wsaBuffer.len = POST_SIZE;

    DWORD dwRecv = 0;
    DWORD dwFlag = 0;
    if (WSARecv(m_socket, &m_RecvOpContext.wsaBuffer, 1, &dwRecv, &dwFlag, 
        (LPWSAOVERLAPPED)&m_RecvOpContext, NULL) != 0)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            return false;
        }
    }

    m_isRecving = true;
    m_bSendOver = false;
    return true;
}

bool TcpConnection::NotifyReadHowMuchBytes(int iTranceCount)
{
    m_vecRecvBuffer.insert(m_vecRecvBuffer.end(), m_tmpRecvBuffer, m_tmpRecvBuffer + iTranceCount);

    return true;
}

bool TcpConnection::UnPack(std::vector<std::shared_ptr<Packet>> &vecPacket)
{
    if (m_vecRecvBuffer.size() == 0)
    {
        return true;
    }

    int32_t	iSize		= m_vecRecvBuffer.size();
    char	*pData		= &m_vecRecvBuffer[0];
    int32_t	iCutSize	= 0;

    while (iSize > (int32_t)sizeof(int32_t))
    {
        int32_t iTrunkLen = *((int32_t*)pData);

        if (iTrunkLen <= (int32_t)sizeof(int32_t)) //非法报文!!!
        {
            /*LOGERROR<<"unpack data failed:client["<<m_llClientHandle<<"] send illegal packet!"
                <<" iTrunkLen:"<<iTrunkLen;*/
            return false;
        }

        if (iSize >= iTrunkLen)
        {						
            std::shared_ptr<Packet> pktPtr = std::make_shared<Packet>();
            pktPtr->assign(pData + sizeof(int32_t), pData + iTrunkLen);
            vecPacket.push_back(pktPtr);

            iSize -= iTrunkLen;
            pData += iTrunkLen;
            iCutSize += iTrunkLen;
        }
        else
        {
            break;
        }
    }

    if (iCutSize > 0)
    {
        m_vecRecvBuffer.erase(m_vecRecvBuffer.begin(), m_vecRecvBuffer.begin() + iCutSize);
    }	

    return true;
}

bool TcpConnection::IsUserClose()
{
    return m_bCloseByUser;
}

bool TcpConnection::IsSendOver()
{
    return m_bSendOver;
}

void TcpConnection::MarkRecving(bool value)
{
    m_isRecving=value;
}

bool TcpConnection::IsRecving()
{
    return m_isRecving;
}

void TcpConnection::MarkSending(bool value)
{
    m_isSending=value;
}

bool TcpConnection::IsSending()
{
    return m_isSending;
}

bool TcpConnection::IsClosing()
{
    return m_isClosing;
}

void TcpConnection::MarkClosing(bool value)
{
    m_isClosing = value;
}

void TcpConnection::Close()
{
    m_bCloseByUser = true;
}
