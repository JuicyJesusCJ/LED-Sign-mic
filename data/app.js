/*
 * LED Sign Control - Main JavaScript
 * Handles WebSocket communication, API calls, and shared functionality
 */

// Global variables
let websocket = null;
let reconnectInterval = null;
let isConnected = false;

// Initialize WebSocket connection
function initializeWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.hostname}:81`;
    
    try {
        websocket = new WebSocket(wsUrl);
        
        websocket.onopen = function(event) {
            console.log('WebSocket connected');
            isConnected = true;
            updateConnectionStatus(true);
            clearInterval(reconnectInterval);
        };
        
        websocket.onmessage = function(event) {
            const data = JSON.parse(event.data);
            handleWebSocketMessage(data);
        };
        
        websocket.onclose = function(event) {
            console.log('WebSocket disconnected');
            isConnected = false;
            updateConnectionStatus(false);
            scheduleReconnect();
        };
        
        websocket.onerror = function(error) {
            console.error('WebSocket error:', error);
            updateConnectionStatus(false);
        };
        
    } catch (error) {
        console.error('Failed to create WebSocket:', error);
        updateConnectionStatus(false);
        scheduleReconnect();
    }
}

// Handle incoming WebSocket messages
function handleWebSocketMessage(data) {
    switch(data.type) {
        case 'status_update':
            if (typeof updateStatusDisplay === 'function') {
                updateStatusDisplay(data);
            }
            break;
            
        case 'audio_data':
            if (typeof updateAudioVisualization === 'function') {
                updateAudioVisualization(data);
            }
            break;
            
        case 'config_changed':
            if (typeof onConfigChanged === 'function') {
                onConfigChanged(data);
            }
            break;
            
        case 'command_result':
            handleCommandResult(data);
            break;
            
        case 'welcome':
            console.log('WebSocket welcome:', data.message);
            break;
            
        default:
            console.log('Unknown message type:', data.type);
    }
}

// Send WebSocket command
function sendWebSocketCommand(command, value) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        const message = {
            command: command,
            value: value
        };
        websocket.send(JSON.stringify(message));
    } else {
        console.error('WebSocket not connected');
        showNotification('Connection error - command not sent', 'error');
    }
}

// Handle command results
function handleCommandResult(data) {
    if (data.status === 'ok') {
        console.log(`Command ${data.command} executed successfully`);
    } else {
        console.error(`Command ${data.command} failed:`, data.error);
        showNotification(`Command failed: ${data.error}`, 'error');
    }
}

// Schedule WebSocket reconnection
function scheduleReconnect() {
    if (reconnectInterval) return;
    
    reconnectInterval = setInterval(() => {
        if (!isConnected) {
            console.log('Attempting to reconnect WebSocket...');
            initializeWebSocket();
        }
    }, 5000);
}

// Update connection status indicator
function updateConnectionStatus(connected) {
    const indicator = document.getElementById('connection-status');
    const text = document.getElementById('connection-text');
    
    if (indicator && text) {
        if (connected) {
            indicator.className = 'connection-indicator';
            text.textContent = 'Connected';
        } else {
            indicator.className = 'connection-indicator error';
            text.textContent = 'Disconnected';
        }
    }
}

// API helper functions
function apiGet(endpoint) {
    return fetch(`/api/${endpoint}`)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            return response.json();
        })
        .catch(error => {
            console.error(`API GET /${endpoint} failed:`, error);
            showNotification(`API Error: ${error.message}`, 'error');
            throw error;
        });
}

function apiPost(endpoint, data = {}) {
    const options = {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
    };
    
    return fetch(`/api/${endpoint}`, options)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            return response.json();
        })
        .catch(error => {
            console.error(`API POST /${endpoint} failed:`, error);
            showNotification(`API Error: ${error.message}`, 'error');
            throw error;
        });
}

function apiPostForm(endpoint, formData) {
    const options = {
        method: 'POST',
        body: formData
    };
    
    return fetch(`/api/${endpoint}`, options)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            return response.json();
        })
        .catch(error => {
            console.error(`API POST /${endpoint} failed:`, error);
            showNotification(`API Error: ${error.message}`, 'error');
            throw error;
        });
}

// Notification system
function showNotification(message, type = 'info', duration = 5000) {
    // Remove any existing notifications
    const existing = document.querySelector('.notification');
    if (existing) {
        existing.remove();
    }
    
    // Create notification element
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.innerHTML = `
        <span class="notification-message">${message}</span>
        <button class="notification-close" onclick="this.parentElement.remove()">×</button>
    `;
    
    // Add styles
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 1rem 1.5rem;
        border-radius: 8px;
        color: white;
        font-weight: 600;
        box-shadow: 0 4px 12px rgba(0,0,0,0.3);
        z-index: 10000;
        max-width: 400px;
        display: flex;
        align-items: center;
        justify-content: space-between;
        gap: 1rem;
        animation: slideIn 0.3s ease-out;
    `;
    
    // Set background color based on type
    const colors = {
        success: '#28a745',
        error: '#dc3545',
        warning: '#ffc107',
        info: '#17a2b8'
    };
    notification.style.backgroundColor = colors[type] || colors.info;
    
    // Add to page
    document.body.appendChild(notification);
    
    // Auto-remove after duration
    if (duration > 0) {
        setTimeout(() => {
            if (notification.parentElement) {
                notification.remove();
            }
        }, duration);
    }
}

// Add notification styles to page
function addNotificationStyles() {
    if (document.getElementById('notification-styles')) return;
    
    const style = document.createElement('style');
    style.id = 'notification-styles';
    style.textContent = `
        @keyframes slideIn {
            from {
                transform: translateX(100%);
                opacity: 0;
            }
            to {
                transform: translateX(0);
                opacity: 1;
            }
        }
        
        .notification-close {
            background: none;
            border: none;
            color: white;
            font-size: 1.2rem;
            cursor: pointer;
            padding: 0;
            width: 20px;
            height: 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            opacity: 0.8;
        }
        
        .notification-close:hover {
            opacity: 1;
        }
    `;
    document.head.appendChild(style);
}

// Utility functions
function formatBytes(bytes) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

function formatDuration(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    
    if (days > 0) {
        return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
        return `${hours}h ${minutes}m`;
    } else if (minutes > 0) {
        return `${minutes}m ${secs}s`;
    } else {
        return `${secs}s`;
    }
}

function hexToRgb(hex) {
    const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
}

function rgbToHex(r, g, b) {
    return "#" + [r, g, b].map(x => {
        const hex = x.toString(16);
        return hex.length === 1 ? "0" + hex : hex;
    }).join("");
}

function hslToRgb(h, s, l) {
    h /= 360;
    s /= 100;
    l /= 100;
    
    const hue2rgb = (p, q, t) => {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1/6) return p + (q - p) * 6 * t;
        if (t < 1/2) return q;
        if (t < 2/3) return p + (q - p) * (2/3 - t) * 6;
        return p;
    };
    
    let r, g, b;
    
    if (s === 0) {
        r = g = b = l;
    } else {
        const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        const p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3);
    }
    
    return {
        r: Math.round(r * 255),
        g: Math.round(g * 255),
        b: Math.round(b * 255)
    };
}

// Debounce function for reducing API calls
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

// Loading state management
function setLoading(element, loading = true) {
    if (typeof element === 'string') {
        element = document.getElementById(element);
    }
    
    if (!element) return;
    
    if (loading) {
        element.classList.add('loading');
        element.style.pointerEvents = 'none';
    } else {
        element.classList.remove('loading');
        element.style.pointerEvents = '';
    }
}

// Form validation helpers
function validateWiFiCredentials(ssid, password) {
    if (!ssid || ssid.trim().length === 0) {
        return { valid: false, message: 'SSID is required' };
    }
    
    if (ssid.length > 32) {
        return { valid: false, message: 'SSID must be 32 characters or less' };
    }
    
    if (password && password.length > 64) {
        return { valid: false, message: 'Password must be 64 characters or less' };
    }
    
    return { valid: true };
}

function validateDeviceName(name) {
    if (!name || name.trim().length === 0) {
        return { valid: false, message: 'Device name is required' };
    }
    
    if (name.length > 32) {
        return { valid: false, message: 'Device name must be 32 characters or less' };
    }
    
    if (!/^[a-zA-Z0-9_-]+$/.test(name)) {
        return { valid: false, message: 'Device name can only contain letters, numbers, hyphens, and underscores' };
    }
    
    return { valid: true };
}

// Navigation helper
function setActiveNavigation(currentPage) {
    document.querySelectorAll('.nav-link').forEach(link => {
        link.classList.remove('active');
        if (link.getAttribute('href') === currentPage || 
            (currentPage === '/' && link.getAttribute('href') === '/')) {
            link.classList.add('active');
        }
    });
}

// Local storage helpers
function saveToLocalStorage(key, value) {
    try {
        localStorage.setItem(key, JSON.stringify(value));
    } catch (error) {
        console.warn('Failed to save to localStorage:', error);
    }
}

function loadFromLocalStorage(key, defaultValue = null) {
    try {
        const item = localStorage.getItem(key);
        return item ? JSON.parse(item) : defaultValue;
    } catch (error) {
        console.warn('Failed to load from localStorage:', error);
        return defaultValue;
    }
}

// Initialize everything when DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
    // Add notification styles
    addNotificationStyles();
    
    // Initialize WebSocket if not already done
    if (!websocket) {
        initializeWebSocket();
    }
    
    // Set active navigation
    const currentPath = window.location.pathname;
    setActiveNavigation(currentPath);
    
    // Show welcome message
    console.log('LED Sign Control Interface Loaded');
});

// Handle page visibility changes to manage WebSocket connection
document.addEventListener('visibilitychange', function() {
    if (document.visibilityState === 'visible') {
        if (!isConnected) {
            initializeWebSocket();
        }
    }
});

// Handle beforeunload to clean up WebSocket
window.addEventListener('beforeunload', function() {
    if (websocket) {
        websocket.close();
    }
});

// Export functions for global use
window.LEDSign = {
    initializeWebSocket,
    sendWebSocketCommand,
    apiGet,
    apiPost,
    apiPostForm,
    showNotification,
    formatBytes,
    formatDuration,
    hexToRgb,
    rgbToHex,
    hslToRgb,
    debounce,
    setLoading,
    validateWiFiCredentials,
    validateDeviceName,
    setActiveNavigation,
    saveToLocalStorage,
    loadFromLocalStorage
}; 