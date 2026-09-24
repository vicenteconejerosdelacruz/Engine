import React from 'react';
import './Boot.css';

export const Boot = () => {
    const handleAnimationEnd = () => {
        if (window.JSBridge) {
            window.JSBridge("BOOT_COMPLETE");
        } else {
            console.warn("[React] JSBridge no encontrado. ¿Estás corriendo en el navegador y no en el motor?");
        }
    };

  return (
    <div className="boot-container">
      <img 
        src="boot/logo.png" 
        alt="Logo Boot" 
        className="boot-logo" 
        onAnimationEnd={handleAnimationEnd}
      />
    </div>
  );
};