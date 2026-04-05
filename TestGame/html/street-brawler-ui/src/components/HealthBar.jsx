export const HealthBar = ({ hp, type }) => {
  const isMain = type === 'main';
  return (
    <div className={`health-bar-bg ${type}`}>
      <div 
        className="health-bar-fill" 
        style={{ height:'100%', width: `${hp}%`, backgroundColor: '#ff4500' }}
      >
        <div className="shine" />
      </div>
    </div>
  );
};